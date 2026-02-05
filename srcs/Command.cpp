#include "Command.hpp"

#include <cassert>
#include <map>
#include <string>

#include "ICommand.hpp"
#include "IServerConfig.hpp"
#include "Response.hpp"
#include "numeric.hpp"

namespace CommandFactory {
class CommandPool {
 public:
  CommandPool() {
    _pool["PASS"] = &_passCmd;
    _pool["NICK"] = &_nickCmd;
    _pool["USER"] = &_userCmd;
    _pool["QUIT"] = &_quitCmd;
    _pool["JOIN"] = &_joinCmd;
    _pool["PART"] = &_partCmd;
    _pool["MODE"] = &_modeCmd;
    _pool["TOPIC"] = &_topicCmd;
    _pool["NAMES"] = &_namesCmd;
    _pool["INVITE"] = &_inviteCmd;
    _pool["KICK"] = &_kickCmd;
    _pool["PRIVMSG"] = &_privmsgCmd;
    _pool["NOTICE"] = &_noticeCmd;
    _pool["DCC"] = &_dccSendCmd;
  };
  const std::map<std::string, ICommand*>& getPool() { return _pool; }

 private:
  std::map<std::string, ICommand*> _pool;
  PassCommand _passCmd;
  NickCommand _nickCmd;
  UserCommand _userCmd;
  QuitCommand _quitCmd;
  JoinCommand _joinCmd;
  PartCommand _partCmd;
  ChannelModeCommand _modeCmd;
  TopicCommand _topicCmd;
  NamesCommand _namesCmd;
  InviteCommand _inviteCmd;
  KickCommand _kickCmd;
  PrivmsgCommand _privmsgCmd;
  NoticeCommand _noticeCmd;
  DccSendCommand _dccSendCmd;
};

const std::map<std::string, ICommand*>& getPool() {
  static CommandPool _pool;
  return _pool.getPool();
}

const ICommand& getCommand(const std::string& cmd) {
  if (getPool().find(cmd) == getPool().end()) {
    assert(0 && "Unexpected Command");
  }
  return *getPool().find(cmd)->second;
}
}  // namespace CommandFactory

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
    // ERR_NEEDMOREPARAMS (461): "<command> :Not enough parameters"
    ctx.requester().send(
        Response::error("461", target, "PASS :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& password = ctx.args()[0];
  const IServerConfig& serverConfig = ctx.serverConfig();

  IClient& client = ctx.requesterClient();
  IRC::Numeric result = client.Authenticate(serverConfig, password);

  switch (result) {
    case IRC::ERR_ALREADYREGISTRED:
      ctx.requester().send(Response::error(
          "462", target, ":Unauthorized command (already registered)"));
      break;
    case IRC::ERR_PASSWDMISMATCH:
      ctx.requester().send(
          Response::error("464", target, ":Password incorrect"));
      break;
    case IRC::DO_NOTHING:
      break;
    default:
      // Unexpected result
      assert(0);  // PASS
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

// Numeric Replies: ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME,
// ERR_NICKNAMEINUSE, ERR_NICKCOLLISION
IRC::Numeric NickCommand::execute(ICommandContext& ctx) const {
  const std::string target = ctx.requesterClient().getNick().empty()
                                 ? "*"
                                 : ctx.requesterClient().getNick();

  if (ctx.args().empty()) {
    // ERR_NONICKNAMEGIVEN (431): ":No nickname given"
    ctx.requester().send(Response::error("431", target, ":No nickname given"));
    return IRC::ERR_NONICKNAMEGIVEN;
  }

  const std::string& newNick = ctx.args()[0];
  IClient& client = ctx.requesterClient();

  IRC::Numeric result = client.setNick(newNick);
  // TODO: Use ClientRegistry to set nickname. The registry will check nickname
  // duplication.
  // IRC::Numeric result = ctx.clients().setNick(current, newNick);

  switch (result) {
    case IRC::ERR_ERRONEUSNICKNAME:
      ctx.requester().send(
          Response::error("432", target, newNick + " :Erroneous nickname"));
      break;

    case IRC::ERR_NICKNAMEINUSE:
      ctx.requester().send(Response::error(
          "433", target, newNick + " :Nickname is already in use"));
      break;

    case IRC::RPL_STRREPLY: {
      const std::vector<std::string>& channels = client.getJoinedChannels();
      const std::string nickChangeMsg = ":" + target + " NICK :" + newNick;
      for (std::vector<std::string>::const_iterator it = channels.begin();
           it != channels.end(); ++it) {
        ctx.channels().broadcast(*it, nickChangeMsg, client.getNick());
      }
      ctx.requester().send(nickChangeMsg);
    } break;

    case IRC::RPL_WELCOME:
      sendWelcomeMessage(ctx);
      break;

    default:
      assert(0);
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
  IRC::Numeric result = client.setUserInfo(username, realname);

  switch (result) {
    case IRC::ERR_ALREADYREGISTRED:
      ctx.requester().send(Response::error(
          "462", target, ":Unauthorized command (already registered)"));
      break;

    case IRC::RPL_WELCOME:
      sendWelcomeMessage(ctx);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

#include "Session.hpp"

// Numeric Replies: None (QUIT has no error cases)
IRC::Numeric QuitCommand::execute(ICommandContext& ctx) const {
  IClient& client = ctx.requesterClient();
  const std::string& nick = client.getNick();
  // Quit message is optional, default to client's nick
  const std::string quitMsg = ctx.args().empty() ? nick : ctx.args()[0];
  const std::string quitNotification = ":" + nick + " QUIT :" + quitMsg;
  const std::vector<std::string>& channels = client.getJoinedChannels();
  for (std::vector<std::string>::const_iterator it = channels.begin();
       it != channels.end(); ++it) {
    ctx.channels().broadcast(*it, quitNotification, nick);
  }
  // Send ERROR to the quitting client
  ctx.requester().send("ERROR :Closing Link: " + nick + " (" + quitMsg + ")");
  // We may need ISessionRegistry
  SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD());

  return IRC::DO_NOTHING;
}

namespace {
void sendChannelNames(ICommandContext& ctx, const std::string& nick,
                      const std::string& channelName) {
  const std::set<std::string>& members = ctx.channels().getClients(channelName);

  // RPL_NAMREPLY (353): "( "=" / "*" / "@" ) <channel> :[ "@" / "+" ]
  // <nick> *( " " [ "@" / "+" ] <nick> )"
  // "=" for public, "*" for private, "@" for secret
  // For now: assume all channels are public
  std::string namesList;
  for (std::set<std::string>::const_iterator it = members.begin();
       it != members.end(); ++it) {
    if (!namesList.empty()) {
      namesList += " ";
    }
    // Prefix with @ if operator
    if (ctx.channels().isClientOp(channelName, *it)) {
      namesList += "@";
    }
    namesList += *it;
  }

  ctx.requester().send(
      Response::build("353", nick, "= " + channelName + " :" + namesList));

  // RPL_ENDOFNAMES (366)
  ctx.requester().send(
      Response::build("366", nick, channelName + " :End of NAMES list"));
}
}  // namespace

// Numeric Replies: ERR_NOSUCHCHANNEL, RPL_NAMREPLY, RPL_ENDOFNAMES
IRC::Numeric NamesCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();

  if (ctx.args().empty()) {
    // No channel specified - list all visible channels
    // For single server: send names for all channels user is in
    const std::vector<std::string>& channels =
        ctx.requesterClient().getJoinedChannels();

    for (std::vector<std::string>::const_iterator it = channels.begin();
         it != channels.end(); ++it) {
      sendChannelNames(ctx, nick, *it);
    }

    // RPL_ENDOFNAMES for wildcard query
    ctx.requester().send(Response::build("366", nick, "* :End of NAMES list"));

    return IRC::RPL_NAMREPLY;
  }

  // Channel(s) specified
  const std::string& channelName = ctx.args()[0];

  if (!ctx.channels().hasChannel(channelName)) {
    // ERR_NOSUCHCHANNEL (403)
    ctx.requester().send(
        Response::error("403", nick, channelName + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }

  sendChannelNames(ctx, nick, channelName);

  return IRC::RPL_NAMREPLY;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL,
// ERR_CHANOPRIVSNEEDED, RPL_NOTOPIC, RPL_TOPIC
IRC::Numeric TopicCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    ctx.requester().send(
        Response::error("461", nick, "TOPIC :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::string& channelName = ctx.args()[0];
  if (!ctx.channels().hasChannel(channelName)) {
    // ERR_NOSUCHCHANNEL (403)
    ctx.requester().send(
        Response::error("403", nick, channelName + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }
  if (!ctx.channels().hasClient(channelName, nick)) {
    // ERR_NOTONCHANNEL (442)
    ctx.requester().send(Response::error(
        "442", nick, channelName + " :You're not on that channel"));
    return IRC::ERR_NOTONCHANNEL;
  }
  if (ctx.args().size() == 1) {
    const std::string& topic = ctx.channels().getTopic(channelName);
    if (topic.empty()) {
      // RPL_NOTOPIC (331)
      ctx.requester().send(
          Response::build("331", nick, channelName + " :No topic is set"));
      return IRC::RPL_NOTOPIC;
    }
    // RPL_TOPIC (332)
    ctx.requester().send(
        Response::build("332", nick, channelName + " :" + topic));
    return IRC::RPL_TOPIC;
  }
  // Set topic
  const std::string& newTopic = ctx.args()[1];
  IRC::Numeric result = ctx.channels().setTopic(channelName, nick, newTopic);
  switch (result) {
    case IRC::ERR_CHANOPRIVSNEEDED:
      ctx.requester().send(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::DO_NOTHING: {
      // Topic changed successfully - broadcast to channel
      const std::string topicMsg =
          ":" + nick + " TOPIC " + channelName + " :" + newTopic;
      ctx.channels().broadcast(channelName, topicMsg);
    } break;
    default:
      assert(0);
      break;
  }

  return result;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL,
// ERR_TOOMANYCHANNELS, ERR_BADCHANNELKEY, ERR_BANNEDFROMCHAN,
// ERR_CHANNELISFULL, ERR_INVITEONLYCHAN, RPL_TOPIC
IRC::Numeric JoinCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    ctx.requester().send(
        Response::error("461", nick, "JOIN :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& channelName = ctx.args()[0];

  // Special case: JOIN 0 - part all channels
  if (channelName == "0") {
    const std::vector<std::string>& channels =
        ctx.requesterClient().getJoinedChannels();
    for (std::vector<std::string>::const_iterator it = channels.begin();
         it != channels.end(); ++it) {
      const std::string partNotification =
          ":" + nick + " PART " + *it + " :" + nick;
      ctx.channels().broadcast(*it, partNotification, nick);
      ctx.channels().partChannel(*it, nick);
    }
    return IRC::DO_NOTHING;
  }

  const std::string key = ctx.args().size() > 1 ? ctx.args()[1] : "";
  IRC::Numeric result =
      ctx.channels().joinChannel(channelName, nick, ctx.clients(), key);

  switch (result) {
    case IRC::ERR_NOSUCHCHANNEL:
      ctx.requester().send(
          Response::error("403", nick, channelName + " :No such channel"));
      break;
    case IRC::ERR_TOOMANYCHANNELS:
      ctx.requester().send(Response::error(
          "405", nick, channelName + " :You have joined too many channels"));
      break;
    case IRC::ERR_BADCHANNELKEY:
      ctx.requester().send(Response::error(
          "475", nick, channelName + " :Cannot join channel (+k)"));
      break;
    case IRC::ERR_INVITEONLYCHAN:
      ctx.requester().send(Response::error(
          "473", nick, channelName + " :Cannot join channel (+i)"));
      break;
    case IRC::ERR_CHANNELISFULL:
      ctx.requester().send(Response::error(
          "471", nick, channelName + " :Cannot join channel (+l)"));
      break;
    case IRC::DO_NOTHING: {
      const std::string joinMsg = ":" + nick + " JOIN :" + channelName;
      ctx.channels().broadcast(channelName, joinMsg);
      const std::string& topic = ctx.channels().getTopic(channelName);
      if (!topic.empty()) {
        // RPL_TOPIC (332)
        ctx.requester().send(
            Response::build("332", nick, channelName + " :" + topic));
      } else {
        // RPL_NOTOPIC (331)
        ctx.requester().send(
            Response::build("331", nick, channelName + " :No topic is set"));
      }
      sendChannelNames(ctx, nick, channelName);
    } break;

    default:
      assert(0);
      break;
  }

  return result;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL
IRC::Numeric PartCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();

  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    ctx.requester().send(
        Response::error("461", nick, "PART :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::string& channelName = ctx.args()[0];
  const std::string partMsg = ctx.args().size() > 1 ? ctx.args()[1] : nick;
  IRC::Numeric result = ctx.channels().partChannel(channelName, nick);

  switch (result) {
    case IRC::ERR_NOSUCHCHANNEL:
      ctx.requester().send(
          Response::error("403", nick, channelName + " :No such channel"));
      break;

    case IRC::ERR_NOTONCHANNEL:
      ctx.requester().send(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      break;

    case IRC::DO_NOTHING: {
      const std::string partNotification =
          ":" + nick + " PART " + channelName + " :" + partMsg;
      ctx.channels().broadcast(channelName, partNotification);
    } break;

    default:
      assert(0);
      break;
  }

  return result;
}

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_CHANOPRIVSNEEDED,
// ERR_USERNOTINCHANNEL, ERR_NOTONCHANNEL, ERR_NOSUCHCHANNEL
IRC::Numeric KickCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();

  if (ctx.args().size() < 2) {
    ctx.requester().send(
        Response::error("461", nick, "KICK :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& channelName = ctx.args()[0];
  const std::string& targetNick = ctx.args()[1];
  const std::string kickMsg = ctx.args().size() > 2 ? ctx.args()[2] : nick;

  IRC::Numeric result =
      ctx.channels().kickChannel(channelName, nick, targetNick);
  assert(0);

  switch (result) {
    case IRC::ERR_NOSUCHCHANNEL:
      ctx.requester().send(
          Response::error("403", nick, channelName + " :No such channel"));
      break;
    case IRC::ERR_NOTONCHANNEL:
      ctx.requester().send(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      break;
    case IRC::ERR_CHANOPRIVSNEEDED:
      ctx.requester().send(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::ERR_USERNOTINCHANNEL:
      ctx.requester().send(Response::error(
          "441", nick,
          targetNick + " " + channelName + " :They aren't on that channel"));
      break;
    case IRC::DO_NOTHING: {
      const std::string kickNotification = ":" + nick + " KICK " + channelName +
                                           " " + targetNick + " :" + kickMsg;
      ctx.channels().broadcast(channelName, kickNotification);
    } break;
    default:
      assert(0);
      break;
  }

  return result;
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
