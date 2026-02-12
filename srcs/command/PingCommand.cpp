#include <string>

#include "ClientManagement.hpp"
#include "Command.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NOORIGIN, ERR_NOSUCHSERVER
IRC::Numeric PingCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);
  if (client == NULL || session == NULL) {
    return IRC::DO_NOTHING;
  }

  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();
  const std::string& serverName = ctx.serverConfig().getServerName();

  // RFC 2812: ERR_NOORIGIN (409) - must have at least one parameter
  if (ctx.args().empty()) {
    session->enqueueMsg(
        Response::error("409", nick, ":No origin specified"));
    return IRC::ERR_NOORIGIN;
  }

  const std::string& token = ctx.args()[0];
  session->enqueueMsg(":" + serverName + " PONG " + serverName + " :" + token +
                      "\r\n");
  return IRC::DO_NOTHING;
}
