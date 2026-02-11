#include <cassert>
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
#include "Validator.hpp"
#include "defs.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_CHANOPRIVSNEEDED,
// ERR_USERNOTINCHANNEL, ERR_NOTONCHANNEL, ERR_NOSUCHCHANNEL
IRC::Numeric KickCommand::execute(ICommandContext& ctx) const {
  const std::string& nick = ctx.requesterClient().getNick();
  ISession& requester = ctx.requester();
  ClientID requesterID = ctx.requesterClient().getID();
  IClient& client = ctx.requesterClient();
  if (!client.isRegistered()) {
    ctx.requester().enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }

  if (ctx.args().size() < 2) {
    requester.enqueueMsg(
        Response::error("461", nick, "KICK :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  std::vector<std::string> channelNames =
      CommandUtility::split(ctx.args()[0], ",");
  std::vector<std::string> targetNicks =
      CommandUtility::split(ctx.args()[1], ",");
  const std::string kickMsg = ctx.args().size() > 2 ? ctx.args()[2] : nick;

  bool isOneToN = (channelNames.size() == 1);
  bool isNToN = (channelNames.size() == targetNicks.size());
  if (!isOneToN && !isNToN) {
    requester.enqueueMsg(
        Response::error("461", nick, "KICK :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  IRC::Numeric lastResult = IRC::DO_NOTHING;
  for (size_t i = 0; i < targetNicks.size(); ++i) {
    std::string currentChanName = isOneToN ? channelNames[0] : channelNames[i];
    std::string currentTargetNick = targetNicks[i];
    if (!Validator::isChannelNameValid(channelNames[i])) {
      requester.enqueueMsg(
          Response::error("476", nick, channelNames[i] + " :Bad Channel Mask"));
      lastResult = IRC::ERR_BADCHANMASK;
      continue;
    }
    IChannel* channel = ChannelManagement::getChannel(currentChanName);
    if (channel == NULL) {
      requester.enqueueMsg(
          Response::error("403", nick, currentChanName + " :No such channel"));
      lastResult = IRC::ERR_NOSUCHCHANNEL;
      continue;
    }
    IClient* targetClient = ClientManagement::getClient(currentTargetNick);
    if (targetClient == NULL || !channel->hasClient(targetClient->getID())) {
      requester.enqueueMsg(
          Response::error("441", nick,
                          currentTargetNick + " " + currentChanName +
                              " :They aren't on that channel"));
      lastResult = IRC::ERR_USERNOTINCHANNEL;
      continue;
    }
    IRC::Numeric result =
        channel->kickClient(requesterID, targetClient->getID());
    switch (result) {
      case IRC::ERR_NOTONCHANNEL:
        requester.enqueueMsg(Response::error(
            "442", nick, currentChanName + " :You're not on that channel"));
        break;
      case IRC::ERR_CHANOPRIVSNEEDED:
        requester.enqueueMsg(Response::error(
            "482", nick, currentChanName + " :You're not channel operator"));
        break;
      case IRC::ERR_USERNOTINCHANNEL:
        requester.enqueueMsg(
            Response::error("441", nick,
                            currentTargetNick + " " + currentChanName +
                                " :They aren't on that channel"));
        break;
      case IRC::RPL_STRREPLY: {
        const std::string kickNotification =
            ":" + nick + " KICK " + currentChanName + " " + currentTargetNick +
            " :" + kickMsg + "\r\n";
        channel->broadcast(kickNotification, targetClient->getID());
        SessionManagement::getSession(targetClient)
            ->enqueueMsg(kickNotification);
      } break;
      default:
        assert(0 && "Unexpected result");
        break;
    }
    lastResult = result;
  }

  return lastResult;
}
