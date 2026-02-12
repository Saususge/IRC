#include <cassert>
#include <cmath>
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
  if (ctx.args().empty()) {
    // ERR_NONICKNAMEGIVEN (431): ":No nickname given"
    session->enqueueMsg(Response::error("431", nick, ":No nickname given"));
    return IRC::ERR_NONICKNAMEGIVEN;
  }

  const std::string& newNick = ctx.args()[0];
  if (ClientManagement::getClient(newNick) != NULL) {
    session->enqueueMsg(
        Response::error("433", nick, newNick + " :Nickname is already in use"));
    return IRC::ERR_NICKNAMEINUSE;
  }
  if (!Validator::isValidNickname(newNick)) {
    session->enqueueMsg(
        Response::error("432", nick, newNick + " :Erroneous nickname"));
    return IRC::ERR_ERRONEUSNICKNAME;
  }

  IRC::Numeric result = client->setNick(newNick);
  switch (result) {
    case IRC::RPL_STRREPLY: {
      const std::set<IChannel*> joinedChannel =
          CommandUtility::getJoinedChannels(clientID);
      std::set<ClientID> idiomatic;
      idiomatic.insert(clientID);
      for (std::set<IChannel*>::const_iterator channelIt =
               joinedChannel.begin();
           channelIt != joinedChannel.end(); ++channelIt) {
        std::set<ClientID> joinedClients = (*channelIt)->getJoinedClients();
        for (std::set<ClientID>::iterator clientIt = joinedClients.begin();
             clientIt != joinedClients.end(); ++clientIt) {
          idiomatic.insert(*clientIt);
        }
      }
      const std::string nickChangeMsg =
          ":" + nick + " NICK :" + newNick + "\r\n";
      for (std::set<ClientID>::iterator clientIt = idiomatic.begin();
           clientIt != idiomatic.end(); ++clientIt) {
        ISession* session = SessionManagement::getSession(*clientIt);
        if (session != NULL) {
          session->enqueueMsg(nickChangeMsg);
        }
      }
    } break;

    case IRC::RPL_WELCOME:
      CommandUtility::sendWelcomeMessageAndRegister(ctx);
      break;

    case IRC::DO_NOTHING:
      break;

    default:
      assert(0 && "Unxpedted result");
      break;
  }

  return result;
}
