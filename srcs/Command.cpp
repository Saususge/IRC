#include "Command.hpp"

#include <cassert>
#include <map>
#include <string>

#include "ICommand.hpp"
#include "IServerConfig.hpp"
#include "Response.hpp"
#include "numeric.hpp"

namespace CommandFactory {
std::map<std::string, ICommand&> pool;
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
  //   ctx.requester().send(Response::build(
  //       "002", nick,
  //       ":Your host is " + ctx.serverConfig().getServerName() +
  //           ", running version " + ctx.serverConfig().getVersion()));
  //   ctx.requester().send(Response::build(
  //       "003", nick,
  //       ":This server was created " +
  //       ctx.serverConfig().getCreationDate()));
  //   ctx.requester().send(
  //       Response::build("004", nick,
  //                       ctx.serverConfig().getServerName() + " " +
  //                           ctx.serverConfig().getVersion() + " " +
  //                           ctx.serverConfig().getUserModes() + " " +
  //                           ctx.serverConfig().getChannelModes()));
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

  IRC::Numeric result = client.setNick(ctx.clients(), newNick);

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
  ctx.requester().disconnect();

  return IRC::DO_NOTHING;
}

namespace {
void sendChannelNames(ICommandContext& ctx, const std::string& nick,
                      const std::string& channelName) {
  const std::set<std::string>& members = ctx.channels().getClients(channelName);

  // RPL_NAMREPLY (353): "( "=" / "*" / "@" ) <channel> :[ "@" / "+" ] <nick> *(
  // " " [ "@" / "+" ] <nick> )"
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

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS,
// ERR_BADCHANNELKEY, ERR_BANNEDFROMCHAN, ERR_CHANNELISFULL, ERR_INVITEONLYCHAN,
// RPL_TOPIC
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
  IRC::Numeric result = ctx.channels().joinChannel(channelName, nick, key);

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