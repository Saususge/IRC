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

IRC::Numeric PartCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();

  if (!client->isRegistered()) {
    session->enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }
  if (ctx.args().empty()) {
    session->enqueueMsg(
        Response::error("461", nick, "PART :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::vector<std::string> channelNames =
      CommandUtility::split(ctx.args()[0]);
  const std::string partMsg = ctx.args().size() > 1 ? ctx.args()[1] : nick;

  IRC::Numeric lastResult = IRC::DO_NOTHING;
  for (size_t i = 0; i < channelNames.size(); ++i) {
    const std::string& channelName = channelNames[i];
    IChannel* channel = ChannelManagement::getChannel(channelName);
    if (channel == NULL) {
      session->enqueueMsg(
          Response::error("403", nick, channelName + " :No such channel"));
      lastResult = IRC::ERR_NOSUCHCHANNEL;
      continue;
    }
    IRC::Numeric result = channel->part(clientID);
    if (result == IRC::ERR_NOTONCHANNEL) {
      session->enqueueMsg(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      lastResult = IRC::ERR_NOTONCHANNEL;
    } else {
      const std::string partNotification =
          ":" + nick + " PART " + channelName + " :" + partMsg + "\r\n";
      channel->broadcast(partNotification, clientID);
      channel->part(clientID);
      session->enqueueMsg(partNotification);
      if (channel->getClientNumber() == 0) {
        ChannelManagement::deleteChannel(channelName);
      }
      lastResult = result;
    }
  }
  return lastResult;
}
