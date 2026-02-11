#include <cassert>

#include "Command.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTRED
IRC::Numeric PassCommand::execute(ICommandContext& ctx) const {
  ISession& requester = ctx.requester();
  const std::string nick = ctx.requesterClient().getNick().empty()
                               ? "*"
                               : ctx.requesterClient().getNick();
  if (ctx.args().empty()) {
    requester.enqueueMsg(
        Response::error("461", nick, "PASS :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  IClient& client = ctx.requesterClient();
  if (client.isAuthenticated()) {
    requester.enqueueMsg(Response::error(
        "462", nick, ":Unauthorized command (already registered)"));
    return IRC::ERR_ALREADYREGISTRED;
  }

  const std::string& password = ctx.args()[0];
  const IServerConfig& serverConfig = ctx.serverConfig();
  if (password != serverConfig.getPassword()) {
    requester.enqueueMsg(
        Response::error("464", nick, "PASS :Password incorrect"));
    requester.enqueueMsg("ERROR :Closing Link: * (Password incorrect)");
    SessionManagement::scheduleForDeletion(requester.getSocketFD(),
                                           ISession::CLOSING);
  }

  IRC::Numeric result = client.Authenticate();
  switch (result) {
    case IRC::DO_NOTHING:
      break;
    default:
      assert(0 && "Unexpected result");
      break;
  }
  return result;
}
