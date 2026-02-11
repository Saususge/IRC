#include <cassert>

#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTRED
IRC::Numeric UserCommand::execute(ICommandContext& ctx) const {
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
  if (ctx.args().size() < 4) {
    // ERR_NEEDMOREPARAMS (461): "<command> :Not enough parameters"
    ctx.requester().enqueueMsg(
        Response::error("461", target, "USER :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& username = ctx.args()[0];
  // args()[1]
  // args()[2]
  const std::string& realname = ctx.args()[3];

  IClient& client = ctx.requesterClient();
  if (client.isRegistered()) {
    ctx.requester().enqueueMsg(Response::error(
        "462", target, ":Unauthorized command (already registered)"));
    return IRC::ERR_ALREADYREGISTRED;
  }

  IRC::Numeric result = client.setUserInfo(username, realname);
  switch (result) {
    case IRC::RPL_WELCOME:
      CommandUtility::sendWelcomeMessage(ctx);
      break;

    case IRC::DO_NOTHING:
      break;

    default:
      assert(0);
      break;
  }

  return result;
}
