#include <cassert>

#include "ClientManagement.hpp"
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
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  const std::string nick = client->getNick().empty() ? "*" : client->getNick();
  if (ctx.args().empty()) {
    session->enqueueMsg(
        Response::error("461", nick, "PASS :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  if (client->isAuthenticated()) {
    session->enqueueMsg(Response::error(
        "462", nick, ":Unauthorized command (already registered)"));
    return IRC::ERR_ALREADYREGISTRED;
  }

  const std::string& password = ctx.args()[0];
  const IServerConfig& serverConfig = ctx.serverConfig();
  if (password != serverConfig.getPassword()) {
    session->enqueueMsg(
        Response::error("464", nick, "PASS :Password incorrect"));
    session->enqueueMsg("ERROR :Closing Link: * (Password incorrect)");
    SessionManagement::scheduleForDeletion(sessionID, ISession::CLOSING);
  }

  IRC::Numeric result = client->Authenticate();
  switch (result) {
    case IRC::DO_NOTHING:
      break;
    default:
      assert(0 && "Unexpected result");
      break;
  }
  return result;
}
