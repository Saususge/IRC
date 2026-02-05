#include "Command.hpp"

#include <cassert>
#include <set>
#include <string>
#include <vector>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "IClientRegistry.hpp"
#include "ICommand.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "Validator.hpp"
#include "defs.hpp"
#include "numeric.hpp"

CommandContext::CommandContext(ISession& sessionRef, IClient& clientRef,
                               IClientRegistry& clientRegistry,
                               IChannelRegistry& channelRegistry,
                               IServerConfig& serverConfig)
    : sessionRef(sessionRef),
      clientRef(clientRef),
      clientRegistry(clientRegistry),
      channelRegistry(channelRegistry),
      serverConfigRef(serverConfig) {}

CommandContext::~CommandContext() {}

const std::string& CommandContext::getCommandType() const {
  return commandType;
}

const std::vector<std::string>& CommandContext::args() const { return argsVec; }

// Use for disconnecting or replying to user
ISession& CommandContext::requester() const { return sessionRef; }

IClient& CommandContext::requesterClient() const { return clientRef; }

IClientRegistry& CommandContext::clients() const { return clientRegistry; }

IChannelRegistry& CommandContext::channels() const { return channelRegistry; }

const IServerConfig& CommandContext::serverConfig() const {
  return serverConfigRef;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTRED
IRC::Numeric PassCommand::execute(ICommandContext& ctx) const {
  const std::string target = ctx.requesterClient().getNick().empty()
                                 ? "*"
                                 : ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    ctx.requester().send(
        Response::error("461", target, "PASS :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& password = ctx.args()[0];
  const IServerConfig& serverConfig = ctx.serverConfig();
  if (password != serverConfig.getPassword()) {
    ctx.requester().send(
        Response::error("464", target, "PASS :Password incorrect"));
    ctx.requester().send("ERROR :Closing Link: * (Password incorrect)");
    SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD());
  }

  IClient& client = ctx.requesterClient();
  IRC::Numeric result = client.Authenticate();
  switch (result) {
    case IRC::ERR_ALREADYREGISTRED:
      ctx.requester().send(Response::error(
          "462", target, ":Unauthorized command (already registered)"));
      break;
    case IRC::DO_NOTHING:
      break;
    default:
      assert(0 && "Unexpected result");
      break;
  }
  return result;
}

namespace {
inline void sendWelcomeMessage(ICommandContext& ctx) {
  const std::string nick = ctx.requesterClient().getNick();
  ctx.requester().send(Response::build(
      "001", nick, ":Welcome to the Internet Relay Network " + nick));
  ctx.requester().send(Response::build(
      "002", nick,
      ":Your host is " + ctx.serverConfig().getServerName() +
          ", running version " + ctx.serverConfig().getVersion()));
  ctx.requester().send(Response::build(
      "003", nick,
      ":This server was created " + ctx.serverConfig().getCreationDate()));
  ctx.requester().send(
      Response::build("004", nick,
                      ctx.serverConfig().getServerName() + " " +
                          ctx.serverConfig().getVersion() + " " +
                          ctx.serverConfig().getUserModes() + " " +
                          ctx.serverConfig().getChannelModes()));
  ctx.requester().send(Response::build("002", nick, ":Your host is "));
  ctx.requester().send(
      Response::build("003", nick, ":This server was created "));
  ctx.requester().send(Response::build("004", nick, ""));
}
};  // namespace

namespace {
std::set<IChannel*> getJoinedChannels(ClientID client) {
  std::set<IChannel*> _joinedChannels = ChannelManagement::getChannels();
  std::set<IChannel*>::iterator it = _joinedChannels.begin();
  while (it != _joinedChannels.end()) {
    if (!(*it)->hasClient(client)) {
      _joinedChannels.erase(it++);
    } else {
      ++it;
    }
  }
  return _joinedChannels;
}
};  // namespace

// Numeric Replies: ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME,
// ERR_NICKNAMEINUSE, ERR_NICKCOLLISION
IRC::Numeric NickCommand::execute(ICommandContext& ctx) const {
  if (ctx.requesterClient().isAuthenticated() == false) {
    ctx.requester().send(
        "ERROR :Closing Link: * (Password required or incorrect)");
    SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD());
  }

  const std::string target = ctx.requesterClient().getNick().empty()
                                 ? "*"
                                 : ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    // ERR_NONICKNAMEGIVEN (431): ":No nickname given"
    ctx.requester().send(Response::error("431", target, ":No nickname given"));
    return IRC::ERR_NONICKNAMEGIVEN;
  }

  const std::string& newNick = ctx.args()[0];
  if (ClientManagement::getClient(newNick) != NULL) {
    ctx.requester().send(Response::error(
        "433", target, newNick + " :Nickname is already in use"));
    return IRC::ERR_NICKNAMEINUSE;
  }
  if (!Validator::isValidNickname(newNick)) {
    ctx.requester().send(
        Response::error("432", target, newNick + " :Erroneous nickname"));
    return IRC::ERR_ERRONEUSNICKNAME;
  }

  IClient& client = ctx.requesterClient();
  IRC::Numeric result = client.setNick(newNick);
  switch (result) {
    case IRC::RPL_STRREPLY: {
      const std::set<IChannel*> joinedChannel =
          getJoinedChannels(client.getID());
      const std::string nickChangeMsg = ":" + target + " NICK :" + newNick;
      for (std::set<IChannel*>::iterator it = joinedChannel.begin();
           it != joinedChannel.end(); ++it) {
        (*it)->broadcast(nickChangeMsg, client.getNick());
      }
      ctx.requester().send(nickChangeMsg);
    } break;

    case IRC::RPL_WELCOME:
      sendWelcomeMessage(ctx);
      break;

    default:
      assert(0 && "Unxpedted result");
      break;
  }

  return result;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTRED
IRC::Numeric UserCommand::execute(ICommandContext& ctx) const {
  const std::string target = ctx.requesterClient().getNick().empty()
                                 ? "*"
                                 : ctx.requesterClient().getNick();

  if (ctx.args().size() < 4) {
    // ERR_NEEDMOREPARAMS (461): "<command> :Not enough parameters"
    ctx.requester().send(
        Response::error("461", target, "USER :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& username = ctx.args()[0];
  // args()[1]
  // args()[2]
  const std::string& realname = ctx.args()[3];

  IClient& client = ctx.requesterClient();
  if (client.isRegistered()) {
    ctx.requester().send(Response::error(
        "462", target, ":Unauthorized command (already registered)"));
    return IRC::ERR_ALREADYREGISTRED;
  }

  IRC::Numeric result = client.setUserInfo(username, realname);
  switch (result) {
    case IRC::RPL_WELCOME:
      sendWelcomeMessage(ctx);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

// Numeric Replies: None (QUIT has no error cases)
IRC::Numeric QuitCommand::execute(ICommandContext& ctx) const {
  IClient& client = ctx.requesterClient();
  const std::string& nick = client.getNick();
  // Quit message is optional, default to client's nick
  const std::string quitMsg = ctx.args().empty() ? nick : ctx.args()[0];
  const std::string quitNotification = ":" + nick + " QUIT :" + quitMsg;
  const std::set<IChannel*> joinedchannels = getJoinedChannels(client.getID());
  for (std::set<IChannel*>::const_iterator it = joinedchannels.begin();
       it != joinedchannels.end(); ++it) {
    (*it)->broadcast(quitNotification, client.getID());
  }
  // Send ERROR to the quitting client
  ctx.requester().send("ERROR :Closing Link: " + nick + " (" + quitMsg + ")");
  SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD());
  return IRC::DO_NOTHING;
}

namespace {
void sendChannelNames(ISession& requester, const std::string& requesterNick,
                      IChannel& channel) {
  const std::string channelName = channel.getChannelName();
  const std::set<ClientID> joinedClients = channel.getJoinedClients();

  std::string namesList;
  namesList.reserve(512);

  for (std::set<ClientID>::const_iterator it = joinedClients.begin();
       it != joinedClients.end(); ++it) {
    std::string nickPrefix = channel.isClientOp(*it) ? "@" : "";
    std::string clientNick = ClientManagement::getClient(*it)->getNick();
    if (!namesList.empty()) namesList += " ";
    namesList += nickPrefix + clientNick;
  }
  if (!namesList.empty()) {
    requester.send(Response::build("353", requesterNick,
                                   "= " + channelName + " :" + namesList));
  }
  requester.send(Response::build("366", requesterNick,
                                 channelName + " :End of NAMES list"));
}

void sendWildcardNames(ISession& requester, const std::string& requesterNick,
                       const std::set<ClientID>& remainClientIDs) {
  if (remainClientIDs.empty()) {
    return;
  }

  std::string namesList;
  namesList.reserve(512);
  for (std::set<ClientID>::const_iterator it = remainClientIDs.begin();
       it != remainClientIDs.end(); ++it) {
    std::string clientNick = ClientManagement::getClient(*it)->getNick();
    if (!namesList.empty()) {
      namesList += " ";
    }
    namesList += clientNick;
  }
  if (!namesList.empty()) {
    requester.send(Response::build("353", requesterNick, "= * :" + namesList));
  }
  requester.send(Response::build("366", requesterNick, "* :End of NAMES list"));
}
};  // namespace

// Numeric Replies: ERR_NOSUCHCHANNEL, RPL_NAMREPLY, RPL_ENDOFNAMES
IRC::Numeric NamesCommand::execute(ICommandContext& ctx) const {
  ISession& requester = ctx.requester();
  const std::string& nick =
      ClientManagement::getClient(requester.getClientID())->getNick();
  if (ctx.args().empty()) {
    // No channel specified - list all visible channels
    // For single server: send names for all channels user is in
    const std::set<IChannel*> allChannels = ChannelManagement::getChannels();
    std::set<ClientID> remainClientIDs = ClientManagement::getClientIDs();
    for (std::set<IChannel*>::const_iterator it = allChannels.begin();
         it != allChannels.end(); ++it) {
      sendChannelNames(requester, nick, **it);
      const std::set<ClientID> joined = (*it)->getJoinedClients();
      for (std::set<ClientID>::const_iterator cIt = joined.begin();
           cIt != joined.end(); ++cIt) {
        remainClientIDs.erase(*cIt);
      }
    }
    // RPL_ENDOFNAMES for wildcard query
    sendWildcardNames(requester, nick, remainClientIDs);
    return IRC::RPL_NAMREPLY;
  }
  // Channel(s) specified
  const std::vector<std::string>& _channelNames = ctx.args();
  for (std::vector<std::string>::const_iterator it = _channelNames.begin();
       it != _channelNames.end(); ++it) {
    IChannel* _channel = ChannelManagement::getChannel(*it);
    if (_channel == NULL) {
      // ERR_NOSUCHCHANNEL (403)
      ctx.requester().send(
          Response::error("403", nick, *it + " :No such channel"));
      continue;
    }
    sendChannelNames(requester, nick, *_channel);
  }
  return IRC::RPL_NAMREPLY;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL,
// ERR_CHANOPRIVSNEEDED, RPL_NOTOPIC, RPL_TOPIC
IRC::Numeric TopicCommand::execute(ICommandContext& ctx) const {
  ISession& requester = ctx.requester();
  ClientID clientID = ctx.requesterClient().getID();
  const std::string& nick = ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    requester.send(
        Response::error("461", nick, "TOPIC :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::string& channelName = ctx.args()[0];
  IChannel* channel = ChannelManagement::getChannel(channelName);
  if (channel == NULL) {
    // ERR_NOSUCHCHANNEL (403)
    requester.send(
        Response::error("403", nick, channelName + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }
  if (!channel->hasClient(clientID)) {
    // ERR_NOTONCHANNEL (442)
    requester.send(Response::error(
        "442", nick, channelName + " :You're not on that channel"));
    return IRC::ERR_NOTONCHANNEL;
  }

  // Send topic
  if (ctx.args().size() == 1) {
    const std::string& topic = channel->getTopic();
    if (topic.empty()) {
      // RPL_NOTOPIC (331)
      requester.send(
          Response::build("331", nick, channelName + " :No topic is set"));
      return IRC::RPL_NOTOPIC;
    }
    // RPL_TOPIC (332)
    requester.send(Response::build("332", nick, channelName + " :" + topic));
    return IRC::RPL_TOPIC;
  }

  // Set topic
  const std::string& newTopic = ctx.args()[1];
  IRC::Numeric result = channel->setTopic(nick, newTopic);
  switch (result) {
    case IRC::ERR_CHANOPRIVSNEEDED:
      requester.send(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::DO_NOTHING: {
      // Topic changed successfully - broadcast to channel
      const std::string topicMsg =
          ":" + nick + " TOPIC " + channelName + " :" + newTopic;
      channel->broadcast(topicMsg, ClientID(-1));
    } break;
    default:
      assert(0 && "Unexpected result");
      break;
  }
  return result;
}

namespace {
std::vector<std::string> split(const std::string& str,
                               const std::string& sep = ",") {
  std::vector<std::string> result;
  if (sep.empty()) {
    result.push_back(str);
    return result;
  }
  std::string::size_type start = 0;
  std::string::size_type end = str.find(sep);
  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + sep.length();
    end = str.find(sep, start);
  }
  result.push_back(str.substr(start));
  return result;
}
};  // namespace

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL,
// ERR_TOOMANYCHANNELS, ERR_BADCHANNELKEY, ERR_BANNEDFROMCHAN,
// ERR_CHANNELISFULL, ERR_INVITEONLYCHAN, RPL_TOPIC
IRC::Numeric JoinCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.requesterClient().getID();
  const std::string& nick = ctx.requesterClient().getNick();
  ISession& requester = ctx.requester();
  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    requester.send(Response::error("461", nick, "JOIN :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  // Special case: JOIN 0 - part all channels
  if (ctx.args()[0] == "0") {
    const std::set<IChannel*> _joinedChannels = getJoinedChannels(clientID);
    for (std::set<IChannel*>::const_iterator it = _joinedChannels.begin();
         it != _joinedChannels.end(); ++it) {
      const std::string partNotification =
          ":" + nick + " PART " + (*it)->getChannelName() + " :" + nick;
      (*it)->broadcast(partNotification, nick);
      (*it)->removeClient(clientID);
      if ((*it)->getClientNumber() == 0) {
        ChannelManagement::deleteChannel((*it)->getChannelName());
      }
    }
    return IRC::DO_NOTHING;
  }

  // Normal case
  const std::vector<std::string> channelNames = split(ctx.args()[0]);
  std::vector<std::string> keys = split(ctx.args()[1]);
  while (keys.size() < channelNames.size()) {
    keys.push_back("");
  }
  IRC::Numeric lastResult;
  for (size_t i = 0; i < channelNames.size(); ++i) {
    if (!Validator::isChannelNameValid(channelNames[i])) {
      requester.send(
          Response::error("476", nick, channelNames[i] + " :Bad Channel Mask"));
      lastResult = IRC::ERR_BADCHANMASK;
      continue;
    }
    IChannel* channel = ChannelManagement::getChannel(channelNames[i]);
    IRC::Numeric result;
    if (channel == NULL) {
      ChannelManagement::createChannel(channelNames[i]);
      channel = ChannelManagement::getChannel(channelNames[i]);
    }
    result = channel->join(clientID, keys[i]);
    switch (result) {
      case IRC::ERR_BADCHANNELKEY:
        requester.send(Response::error(
            "475", nick, channelNames[i] + " :Cannot join channel (+k)"));
        break;
      case IRC::ERR_INVITEONLYCHAN:
        requester.send(Response::error(
            "473", nick, channelNames[i] + " :Cannot join channel (+i)"));
        break;
      case IRC::ERR_CHANNELISFULL:
        requester.send(Response::error(
            "471", nick, channelNames[i] + " :Cannot join channel (+l)"));
        break;
      case IRC::RPL_STRREPLY: {
        // Send topic and names
        const std::string joinMsg = ":" + nick + " JOIN :" + channelNames[i];
        channel->broadcast(joinMsg, ClientID(-1));
        const std::string& topic = channel->getTopic();
        if (!topic.empty()) {
          requester.send(
              Response::build("332", nick, channelNames[i] + " :" + topic));
        } else {
          requester.send(Response::build(
              "331", nick, channelNames[i] + " :No topic is set"));
        }
        sendChannelNames(requester, nick, *channel);
      } break;
      default:
        assert(0 && "Unexpected result");
        break;
    }
    lastResult = result;
  }
  return lastResult;
}

IRC::Numeric PartCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  ISession& requester = ctx.requester();
  ClientID clientID = ctx.requesterClient().getID();
  if (ctx.args().empty()) {
    requester.send(Response::error("461", nick, "PART :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::vector<std::string> channelNames = split(ctx.args()[0]);
  const std::string partMsg = ctx.args().size() > 1 ? ctx.args()[1] : nick;

  IRC::Numeric lastResult = IRC::DO_NOTHING;
  for (size_t i = 0; i < channelNames.size(); ++i) {
    const std::string& channelName = channelNames[i];
    IChannel* channel = ChannelManagement::getChannel(channelName);
    if (channel == NULL) {
      requester.send(
          Response::error("403", nick, channelName + " :No such channel"));
      lastResult = IRC::ERR_NOSUCHCHANNEL;
      continue;
    }
    IRC::Numeric result = channel->part(clientID);
    if (result == IRC::ERR_NOTONCHANNEL) {
      requester.send(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      lastResult = IRC::ERR_NOTONCHANNEL;
    } else {
      const std::string partNotification =
          ":" + nick + " PART " + channelName + " :" + partMsg;
      channel->broadcast(partNotification, clientID);
      requester.send(partNotification);
      if (channel->getClientNumber() == 0) {
        ChannelManagement::deleteChannel(channelName);
      }
      lastResult = result;
    }
  }
  return lastResult;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_CHANOPRIVSNEEDED,
// ERR_USERNOTINCHANNEL, ERR_NOTONCHANNEL, ERR_NOSUCHCHANNEL
IRC::Numeric KickCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  ISession& requester = ctx.requester();
  ClientID requesterID = ctx.requesterClient().getID();

  if (ctx.args().size() < 2) {
    requester.send(Response::error("461", nick, "KICK :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  std::vector<std::string> channelNames = split(ctx.args()[0], ",");
  std::vector<std::string> targetNicks = split(ctx.args()[1], ",");
  const std::string kickMsg = ctx.args().size() > 2 ? ctx.args()[2] : nick;

  bool isOneToN = (channelNames.size() == 1);
  bool isNToN = (channelNames.size() == targetNicks.size());
  if (!isOneToN && !isNToN) {
    requester.send(Response::error("461", nick, "KICK :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  IRC::Numeric lastResult = IRC::DO_NOTHING;
  for (size_t i = 0; i < targetNicks.size(); ++i) {
    std::string currentChanName = isOneToN ? channelNames[0] : channelNames[i];
    std::string currentTargetNick = targetNicks[i];
    if (!Validator::isChannelNameValid(channelNames[i])) {
      requester.send(
          Response::error("476", nick, channelNames[i] + " :Bad Channel Mask"));
      lastResult = IRC::ERR_BADCHANMASK;
      continue;
    }
    IChannel* channel = ChannelManagement::getChannel(currentChanName);
    if (channel == NULL) {
      requester.send(
          Response::error("403", nick, currentChanName + " :No such channel"));
      lastResult = IRC::ERR_NOSUCHCHANNEL;
      continue;
    }
    IClient* targetClient = ClientManagement::getClient(currentTargetNick);
    if (targetClient == NULL || !channel->hasClient(targetClient->getID())) {
      requester.send(Response::error("441", nick,
                                     currentTargetNick + " " + currentChanName +
                                         " :They aren't on that channel"));
      lastResult = IRC::ERR_USERNOTINCHANNEL;
      continue;
    }
    IRC::Numeric result =
        channel->kickClient(requesterID, targetClient->getID());
    switch (result) {
      case IRC::ERR_NOTONCHANNEL:
        requester.send(Response::error(
            "442", nick, currentChanName + " :You're not on that channel"));
        break;
      case IRC::ERR_CHANOPRIVSNEEDED:
        requester.send(Response::error(
            "482", nick, currentChanName + " :You're not channel operator"));
        break;
      case IRC::ERR_USERNOTINCHANNEL:
        requester.send(Response::error("441", nick,
                                       currentTargetNick + " " +
                                           currentChanName +
                                           " :They aren't on that channel"));
        break;
      case IRC::RPL_STRREPLY: {
        const std::string kickNotification = ":" + nick + " KICK " +
                                             currentChanName + " " +
                                             currentTargetNick + " :" + kickMsg;
        channel->broadcast(kickNotification, targetClient->getID());
        SessionManagement::getSession(targetClient)->send(kickNotification);
      } break;
      default:
        assert(0 && "Unexpected result");
        break;
    }
    lastResult = result;
  }

  return lastResult;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL,
// ERR_USERONCHANNEL, ERR_CHANOPRIVSNEEDED
IRC::Numeric InviteCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();

  if (ctx.args().size() < 2) {
    ctx.requester().send(
        Response::error("461", nick, "INVITE :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& targetNick = ctx.args()[0];
  const std::string& channelName = ctx.args()[1];

  IRC::Numeric result =
      ctx.channels().addToInviteList(channelName, nick, targetNick);
  switch (result) {
    case IRC::ERR_NOTONCHANNEL:
      ctx.requester().send(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      break;
    case IRC::ERR_CHANOPRIVSNEEDED:
      ctx.requester().send(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::ERR_USERONCHANNEL:
      ctx.requester().send(Response::error(
          "443", nick,
          targetNick + " " + channelName + " :is already on channel"));
      break;
    case IRC::RPL_INVITING: {
      ctx.requester().send(
          Response::build("341", nick, targetNick + " " + channelName));
      const std::string inviteNotification =
          ":" + nick + " INVITE " + targetNick + " :" + channelName;
      ctx.clients().send(targetNick, inviteNotification);
    } break;

    default:
      assert(0);
      break;
  }

  return result;
}

namespace {
struct ModeChange {
  bool isAdding;
  char mode;
  std::string param;
};

std::vector<ModeChange> parseModeString(
    const std::string& modeString, const std::vector<std::string>& params) {
  std::vector<ModeChange> changes;
  bool isAdding = true;
  size_t paramIndex = 0;

  for (size_t i = 0; i < modeString.size(); ++i) {
    char c = modeString[i];

    if (c == '+') {
      isAdding = true;
      continue;
    }
    if (c == '-') {
      isAdding = false;
      continue;
    }

    ModeChange change;
    change.isAdding = isAdding;
    change.mode = c;

    // Modes that require parameters
    if (c == 'o' || c == 'k' || c == 'l') {
      if (paramIndex < params.size()) {
        change.param = params[paramIndex++];
      }
    }

    changes.push_back(change);
  }

  return changes;
}

IChannel::IChannelMode charToMode(char c) {
  switch (c) {
    case 'i':
      return IChannel::MINVITE;
    case 't':
      return IChannel::MTOPIC;
    case 'k':
      return IChannel::MKEY;
    case 'o':
      return IChannel::MOP;
    case 'l':
      return IChannel::MLIMIT;
    default:
      assert(0);
      return IChannel::MLIMIT;
  }
}
}  // namespace

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_CHANOPRIVSNEEDED,
// ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL, ERR_KEYSET, ERR_UNKNOWNMODE,
// ERR_USERSDONTMATCH, RPL_CHANNELMODEIS, RPL_BANLIST, RPL_ENDOFBANLIST
IRC::Numeric ChannelModeCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    ctx.requester().send(
        Response::error("461", nick, "MODE :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::string& target = ctx.args()[0];
  if (!(target[0] == '#' || target[0] == '&' || target[0] == '+')) {
    ctx.requester().send(
        Response::error("502", nick, ":Can't change mode for other users"));
    return IRC::ERR_USERSDONTMATCH;
  }
  if (!ctx.channels().hasChannel(target)) {
    ctx.requester().send(
        Response::error("403", nick, target + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }

  // Query mode
  if (ctx.args().size() == 1) {
    const std::map<std::string, IChannel*>& channels =
        ctx.channels().getChannels();
    std::map<std::string, IChannel*>::const_iterator it = channels.find(target);
    if (it == channels.end()) {
      ctx.requester().send(
          Response::error("403", nick, target + " :No such channel"));
      return IRC::ERR_NOSUCHCHANNEL;
    }

    // TODO: Should we implement RPL_CHANNELMODEIS?
    // const std::string modes = it->second->getModeString();
    // ctx.requester().send(Response::build("324", nick, target + " " + modes));
    return IRC::RPL_CHANNELMODEIS;
  }

  // Set mode
  const std::string& modeString = ctx.args()[1];
  std::vector<std::string> modeParams;
  for (size_t i = 2; i < ctx.args().size(); ++i) {
    modeParams.push_back(ctx.args()[i]);
  }

  std::vector<ModeChange> changes = parseModeString(modeString, modeParams);
  std::string appliedModes;
  std::string appliedParams;
  bool lastWasAdding = true;

  for (std::vector<ModeChange>::const_iterator it = changes.begin();
       it != changes.end(); ++it) {
    const ModeChange& change = *it;
    IChannel::IChannelMode mode = charToMode(change.mode);
    IRC::Numeric result;

    if (change.mode == 'o') {
      result = change.isAdding
                   ? ctx.channels().setClientOp(target, nick, change.param)
                   : ctx.channels().unsetClientOp(target, nick, change.param);

    } else {
      result =
          change.isAdding
              ? ctx.channels().addMode(target, nick, mode, change.param)
              : ctx.channels().removeMode(target, nick, mode, change.param);
    }

    switch (result) {
      case IRC::ERR_NOTONCHANNEL:
        ctx.requester().send(Response::error(
            "442", nick, target + " :You're not on that channel"));
        return IRC::ERR_NOTONCHANNEL;

      case IRC::ERR_CHANOPRIVSNEEDED:
        ctx.requester().send(Response::error(
            "482", nick, target + " :You're not channel operator"));
        return IRC::ERR_CHANOPRIVSNEEDED;

      case IRC::DO_NOTHING:
        if (lastWasAdding != change.isAdding) {
          appliedModes += change.isAdding ? '+' : '-';
          lastWasAdding = change.isAdding;
        }
        appliedModes += change.mode;
        if (!change.param.empty()) {
          if (!appliedParams.empty()) appliedParams += " ";
          appliedParams += change.param;
        }
        break;

      default:
        assert(0);
        break;
    }
  }

  if (!appliedModes.empty()) {
    std::string modeNotification =
        ":" + nick + " MODE " + target + " " + appliedModes;
    if (!appliedParams.empty()) {
      modeNotification += " " + appliedParams;
    }
    ctx.channels().broadcast(target, modeNotification);
  }

  return IRC::DO_NOTHING;
}

// Numeric Replies: ERR_NORECIPIENT, ERR_NOTEXTTOSEND, ERR_CANNOTSENDTOCHAN,
// ERR_NOTOPLEVEL, ERR_WILDTOPLEVEL, ERR_TOOMANYTARGETS, ERR_NOSUCHNICK,
// ERR_NOSUCHSERVER
IRC::Numeric PrivmsgCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    ctx.requester().send(
        Response::error("411", nick, ":No recipient given (PRIVMSG)"));
    return IRC::ERR_NORECIPIENT;
  }
  if (ctx.args().size() < 2 || ctx.args()[1].empty()) {
    ctx.requester().send(Response::error("412", nick, ":No text to send"));
    return IRC::ERR_NOTEXTTOSEND;
  }
  const std::string& target = ctx.args()[0];
  const std::string& message = ctx.args()[1];
  const std::string privmsgNotification =
      ":" + nick + " PRIVMSG " + target + " :" + message;
  if (target[0] == '#' || target[0] == '&' || target[0] == '+') {
    // Channel message
    if (!ctx.channels().hasChannel(target)) {
      ctx.requester().send(
          Response::error("403", nick, target + " :No such channel"));
      return IRC::ERR_NOSUCHCHANNEL;
    }
    ctx.channels().broadcast(target, privmsgNotification, nick);
    return IRC::DO_NOTHING;
  }

  // Private message to user
  if (!ctx.clients().hasClient(target)) {
    ctx.requester().send(
        Response::error("401", nick, target + " :No such nick/channel"));
    return IRC::ERR_NOSUCHNICK;
  }
  ctx.clients().send(target, privmsgNotification);
  return IRC::DO_NOTHING;
}

// Numeric Replies: Same as PRIVMSG except no automatic replies
IRC::Numeric NoticeCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  // NOTICE never returns errors
  if (ctx.args().empty() || ctx.args().size() < 2 || ctx.args()[1].empty()) {
    return IRC::DO_NOTHING;
  }
  const std::string& target = ctx.args()[0];
  const std::string& message = ctx.args()[1];
  const std::string noticeNotification =
      ":" + nick + " NOTICE " + target + " :" + message;

  if (target[0] == '#' || target[0] == '&' || target[0] == '+') {
    // Channel notice
    if (!ctx.channels().hasChannel(target) ||
        !ctx.channels().hasClient(target, nick)) {
      return IRC::DO_NOTHING;
    }

    ctx.channels().broadcast(target, noticeNotification, nick);
    return IRC::DO_NOTHING;
  }

  // Private notice to user
  if (!ctx.clients().hasClient(target)) {
    return IRC::DO_NOTHING;
  }
  ctx.clients().send(target, noticeNotification);
  return IRC::DO_NOTHING;
}
