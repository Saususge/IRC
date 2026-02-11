#include <cassert>
#include <set>
#include <string>

#include "ClientManagement.hpp"
#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "Validator.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME,
// ERR_NICKNAMEINUSE, ERR_NICKCOLLISION
IRC::Numeric NickCommand::execute(ICommandContext& ctx) const {
  if (ctx.requesterClient().isAuthenticated() == false) {
    ctx.requester().enqueueMsg(
        "ERROR :Closing Link: * (Password required or incorrect)\r\n");
    SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD(),
                                           ISession::CLOSING);
    return IRC::ERR_REGISTERBEFOREPASS;
  }

  const std::string target = ctx.requesterClient().getNick().empty()
                                 ? "*"
                                 : ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    // ERR_NONICKNAMEGIVEN (431): ":No nickname given"
    ctx.requester().enqueueMsg(
        Response::error("431", target, ":No nickname given"));
    return IRC::ERR_NONICKNAMEGIVEN;
  }

  const std::string& newNick = ctx.args()[0];
  if (ClientManagement::getClient(newNick) != NULL) {
    ctx.requester().enqueueMsg(Response::error(
        "433", target, newNick + " :Nickname is already in use"));
    return IRC::ERR_NICKNAMEINUSE;
  }
  if (!Validator::isValidNickname(newNick)) {
    ctx.requester().enqueueMsg(
        Response::error("432", target, newNick + " :Erroneous nickname"));
    return IRC::ERR_ERRONEUSNICKNAME;
  }

  IClient& client = ctx.requesterClient();
  IRC::Numeric result = client.setNick(newNick);
  switch (result) {
    case IRC::RPL_STRREPLY: {
      const std::set<IChannel*> joinedChannel =
          CommandUtility::getJoinedChannels(client.getID());
      const std::string nickChangeMsg =
          ":" + target + " NICK :" + newNick + "\r\n";
      for (std::set<IChannel*>::iterator it = joinedChannel.begin();
           it != joinedChannel.end(); ++it) {
        (*it)->broadcast(nickChangeMsg, ClientID(-1));
      }
    } break;

    case IRC::RPL_WELCOME:
      CommandUtility::sendWelcomeMessage(ctx);
      break;

    case IRC::DO_NOTHING:
      break;

    default:
      assert(0 && "Unxpedted result");
      break;
  }

  return result;
}
