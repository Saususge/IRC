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

    case IRC::RPL_WELCOME: {
      const std::string nick = client.getNick();
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
    } break;

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

    case IRC::RPL_WELCOME: {
      // TODO: welcome message
      const std::string& nick = client.getNick();
      ctx.requester().send(Response::build(
          "001", nick, ":Welcome to the Internet Relay Network " + nick));
      ctx.requester().send(Response::build("002", nick, ":Your host is "));
      ctx.requester().send(
          Response::build("003", nick, ":This server was created "));
      ctx.requester().send(Response::build("004", nick, ""));
    } break;

    default:
      assert(0);
      break;
  }

  return result;
}
