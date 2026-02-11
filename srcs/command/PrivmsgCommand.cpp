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
#include "numeric.hpp"

// Numeric Replies: ERR_NORECIPIENT, ERR_NOTEXTTOSEND, ERR_CANNOTSENDTOCHAN,
// ERR_NOTOPLEVEL, ERR_WILDTOPLEVEL, ERR_TOOMANYTARGETS, ERR_NOSUCHNICK,
// ERR_NOSUCHSERVER
IRC::Numeric PrivmsgCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  ISession& requester = ctx.requester();
  {
    IClient& client = ctx.requesterClient();
    if (!client.isRegistered()) {
      ctx.requester().enqueueMsg(
          Response::error("451", nick, ": You have not registered"));
      return IRC::ERR_NOTREGISTERED;
    }
  }
  if (ctx.args().empty()) {
    requester.enqueueMsg(
        Response::error("411", nick, ":No recipient given (PRIVMSG)"));
    return IRC::ERR_NORECIPIENT;
  }
  if (ctx.args().size() < 2 || ctx.args()[1].empty()) {
    requester.enqueueMsg(Response::error("412", nick, ":No text to send"));
    return IRC::ERR_NOTEXTTOSEND;
  }
  const std::string& target = ctx.args()[0];
  const std::string& message = ctx.args()[1];
  const std::string privmsgNotification =
      ":" + nick + " PRIVMSG " + target + " :" + message + "\r\n";
  if (target[0] == '#' || target[0] == '&' || target[0] == '+') {
    // Channel message
    IChannel* channel = ChannelManagement::getChannel(target);
    if (channel == NULL) {
      requester.enqueueMsg(
          Response::error("403", nick, target + " :No such channel"));
      return IRC::ERR_NOSUCHCHANNEL;
    }
    channel->broadcast(privmsgNotification, ctx.requesterClient().getID());
    return IRC::DO_NOTHING;
  }

  // Private message to user
  IClient* client = ClientManagement::getClient(target);
  if (client == NULL) {
    requester.enqueueMsg(
        Response::error("401", nick, target + " :No such nick/channel"));
    return IRC::ERR_NOSUCHNICK;
  }
  SessionManagement::getSession(client)->enqueueMsg(privmsgNotification);
  return IRC::DO_NOTHING;
}
