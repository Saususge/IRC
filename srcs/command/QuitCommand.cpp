#include <set>
#include <string>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "SessionManagement.hpp"
#include "defs.hpp"
#include "numeric.hpp"

// Numeric Replies: None (QUIT has no error cases)
IRC::Numeric QuitCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);
  if (client == NULL || session == NULL) {
    return IRC::DO_NOTHING;
  }
  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();
  // Quit message is optional, default to client's nick
  const std::string quitMsg = ctx.args().empty() ? nick : ctx.args()[0];

  if (client->isRegistered()) {
    const std::string prefix = CommandUtility::getClientPrefix(clientID);
    const std::string quitNotification =
        ":" + prefix + " QUIT :" + quitMsg + "\r\n";
    const std::set<IChannel*> joinedchannels =
        CommandUtility::getJoinedChannels(client->getID());
    for (std::set<IChannel*>::const_iterator it = joinedchannels.begin();
         it != joinedchannels.end(); ++it) {
      (*it)->part(client->getID());
      (*it)->broadcast(quitNotification, client->getID());
      if ((*it)->getClientNumber() == 0) {
        ChannelManagement::deleteChannel((*it)->getChannelName());
      }
    }
  }
  // Send ERROR to the quitting client
  session->enqueueMsg("ERROR :Closing Link: " + nick + " (" + quitMsg +
                      ")\r\n");
  SessionManagement::scheduleForDeletion(sessionID, ISession::CLOSING);
  return IRC::DO_NOTHING;
}
