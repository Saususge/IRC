#include <cassert>
#include <string>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "Command.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "defs.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL,
// ERR_USERONCHANNEL, ERR_CHANOPRIVSNEEDED
IRC::Numeric InviteCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  if (client == NULL || session == NULL) {
    return IRC::DO_NOTHING;
  }

  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();
  ClientID requesterID = clientID;

  if (!client->isRegistered()) {
    session->enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }

  if (ctx.args().size() < 2) {
    session->enqueueMsg(
        Response::error("461", nick, "INVITE :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& targetNick = ctx.args()[0];
  IClient* targetClient = ClientManagement::getClient(targetNick);
  if (targetClient == NULL) {
    session->enqueueMsg(
        Response::error("401", nick, targetNick + " :No such nick"));
    return IRC::ERR_NOSUCHNICK;
  }
  const std::string& channelName = ctx.args()[1];
  IChannel* channel = ChannelManagement::getChannel(channelName);
  if (channel == NULL) {
    const std::string inviteNotification =
        ":" + nick + " INVITE " + targetNick + " :" + channelName + "\r\n";
    session->enqueueMsg(inviteNotification);
    ISession* targetSession = SessionManagement::getSession(targetClient);
    targetSession->enqueueMsg(inviteNotification);
    return IRC::RPL_INVITING;
  }

  IRC::Numeric result = channel->addToInviteList(
      requesterID, ClientManagement::getClientID(targetClient));
  switch (result) {
    case IRC::ERR_NOTONCHANNEL:
      session->enqueueMsg(Response::error(
          "442", nick, channelName + " :You're not on that channel"));
      break;
    case IRC::ERR_CHANOPRIVSNEEDED:
      session->enqueueMsg(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::ERR_USERONCHANNEL:
      session->enqueueMsg(Response::error(
          IRC::ERR_USERONCHANNEL, nick,
          targetNick + " " + channelName + " :is already on channel"));
      break;
    case IRC::RPL_INVITING: {
      session->enqueueMsg(
          Response::build("341", nick, targetNick + " " + channelName));
      const std::string inviteNotification =
          ":" + nick + " INVITE " + targetNick + " :" + channelName + "\r\n";
      // TODO: why above message sending to requester?
      session->enqueueMsg(inviteNotification);
      ISession* targetSession = SessionManagement::getSession(targetClient);
      targetSession->enqueueMsg(inviteNotification);
    } break;

    default:
      assert(0);
      break;
  }

  return result;
}
