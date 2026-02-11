#include <set>
#include <string>

#include "ChannelManagement.hpp"
#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "numeric.hpp"

// Numeric Replies: None (QUIT has no error cases)
IRC::Numeric QuitCommand::execute(ICommandContext& ctx) const {
  IClient& client = ctx.requesterClient();
  const std::string& nick = client.getNick().empty() ? "*" : client.getNick();
  if (!client.isRegistered()) {
    ctx.requester().enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }
  // Quit message is optional, default to client's nick
  const std::string quitMsg = ctx.args().empty() ? nick : ctx.args()[0];
  const std::string quitNotification =
      ":" + nick + " QUIT :" + quitMsg + "\r\n";
  const std::set<IChannel*> joinedchannels =
      CommandUtility::getJoinedChannels(client.getID());
  for (std::set<IChannel*>::const_iterator it = joinedchannels.begin();
       it != joinedchannels.end(); ++it) {
    (*it)->part(client.getID());
    (*it)->broadcast(quitNotification, client.getID());
    if ((*it)->getClientNumber() == 0) {
      ChannelManagement::deleteChannel((*it)->getChannelName());
    }
  }
  // Send ERROR to the quitting client
  ctx.requester().enqueueMsg("ERROR :Closing Link: " + nick + " (" + quitMsg +
                             ")");
  SessionManagement::scheduleForDeletion(ctx.requester().getSocketFD(),
                                         ISession::CLOSING);
  return IRC::DO_NOTHING;
}
