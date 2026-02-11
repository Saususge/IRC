#include <cassert>

#include "ClientManagement.hpp"
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
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  if (client->isAuthenticated() == false) {
    session->enqueueMsg(
        "ERROR :Closing Link: * (Password required or incorrect)\r\n");
    SessionManagement::scheduleForDeletion(sessionID, ISession::CLOSING);
    return IRC::ERR_REGISTERBEFOREPASS;
  }

  const std::string nick = client->getNick().empty() ? "*" : client->getNick();
  if (ctx.args().size() < 4) {
    // ERR_NEEDMOREPARAMS (461): "<command> :Not enough parameters"
    session->enqueueMsg(
        Response::error("461", nick, "USER :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& username = ctx.args()[0];
  // args()[1]
  // args()[2]
  const std::string& realname = ctx.args()[3];

  if (client->isRegistered()) {
    session->enqueueMsg(Response::error(
        "462", nick, ":Unauthorized command (already registered)"));
    return IRC::ERR_ALREADYREGISTRED;
  }

  IRC::Numeric result = client->setUserInfo(username, realname);
  switch (result) {
    case IRC::RPL_WELCOME:
      CommandUtility::sendWelcomeMessageAndRegister(ctx);
      break;

    case IRC::DO_NOTHING:
      break;

    default:
      assert(0);
      break;
  }

  return result;
}
