#include <set>
#include <string>
#include <vector>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "defs.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NOSUCHCHANNEL, RPL_NAMREPLY, RPL_ENDOFNAMES
IRC::Numeric NamesCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  const std::string nick = client->getNick().empty() ? "*" : client->getNick();

  if (!client->isRegistered()) {
    session->enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }
  if (ctx.args().empty()) {
    // No channel specified - list all visible channels
    // For single server: send names for all channels user is in
    const std::set<IChannel*> allChannels = ChannelManagement::getChannels();
    std::set<ClientID> remainClientIDs = ClientManagement::getClientIDs();
    for (std::set<IChannel*>::const_iterator it = allChannels.begin();
         it != allChannels.end(); ++it) {
      CommandUtility::sendChannelNames(*session, nick, **it);
      const std::set<ClientID> joined = (*it)->getJoinedClients();
      for (std::set<ClientID>::const_iterator cIt = joined.begin();
           cIt != joined.end(); ++cIt) {
        remainClientIDs.erase(*cIt);
      }
    }
    // RPL_ENDOFNAMES for wildcard query
    CommandUtility::sendWildcardNames(*session, nick, remainClientIDs);
    return IRC::RPL_NAMREPLY;
  }
  // Channel(s) specified
  const std::vector<std::string>& _channelNames = ctx.args();
  for (std::vector<std::string>::const_iterator it = _channelNames.begin();
       it != _channelNames.end(); ++it) {
    IChannel* _channel = ChannelManagement::getChannel(*it);
    if (_channel == NULL) {
      // ERR_NOSUCHCHANNEL (403)
      session->enqueueMsg(
          Response::error("403", nick, *it + " :No such channel"));
      continue;
    }
    CommandUtility::sendChannelNames(*session, nick, *_channel);
  }
  return IRC::RPL_NAMREPLY;
}
