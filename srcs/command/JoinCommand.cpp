#include <cassert>
#include <iostream>
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
#include "Validator.hpp"
#include "defs.hpp"
#include "numeric.hpp"

// Numeric Replies: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL,
// ERR_TOOMANYCHANNELS, ERR_BADCHANNELKEY, ERR_BANNEDFROMCHAN,
// ERR_CHANNELISFULL, ERR_INVITEONLYCHAN, RPL_TOPIC
IRC::Numeric JoinCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  IClient* client = ClientManagement::getClient(clientID);
  SessionID sessionID = ctx.sessionID();
  ISession* session = SessionManagement::getSession(sessionID);

  if (client == NULL || session == NULL) {
    return IRC::DO_NOTHING;
  }

  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();

  if (!client->isRegistered()) {
    session->enqueueMsg(
        Response::error("451", nick, ": You have not registered"));
    return IRC::ERR_NOTREGISTERED;
  }
  if (ctx.args().empty()) {
    // ERR_NEEDMOREPARAMS (461)
    session->enqueueMsg(
        Response::error("461", nick, "JOIN :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  // Special case: JOIN 0 - part all channels
  if (ctx.args()[0] == "0") {
    const std::set<IChannel*> _joinedChannels =
        CommandUtility::getJoinedChannels(clientID);
    for (std::set<IChannel*>::const_iterator it = _joinedChannels.begin();
         it != _joinedChannels.end(); ++it) {
      const std::string partNotification = ":" + nick + " PART " +
                                           (*it)->getChannelName() + " :" +
                                           nick + "\r\n";
      (*it)->removeClient(clientID);
      (*it)->broadcast(partNotification, clientID);
      session->enqueueMsg(partNotification);
      if ((*it)->getClientNumber() == 0) {
        ChannelManagement::deleteChannel((*it)->getChannelName());
      }
    }
    return IRC::DO_NOTHING;
  }

  // Normal case
  const std::vector<std::string> channelNames =
      CommandUtility::split(ctx.args()[0]);
  std::vector<std::string> keys;
  if (ctx.args().size() > 1)
    keys = CommandUtility::split(ctx.args()[1]);
  else
    keys = std::vector<std::string>();
  while (keys.size() < channelNames.size()) {
    keys.push_back("");
  }
  IRC::Numeric lastResult;
  for (size_t i = 0; i < channelNames.size(); ++i) {
    if (!Validator::isChannelNameValid(channelNames[i])) {
      session->enqueueMsg(
          Response::error("476", nick, channelNames[i] + " :Bad Channel Mask"));
      lastResult = IRC::ERR_BADCHANMASK;
      continue;
    }
    IChannel* channel = ChannelManagement::getChannel(channelNames[i]);
    IRC::Numeric result;
    if (channel == NULL) {
      ChannelManagement::createChannel(channelNames[i]);
      channel = ChannelManagement::getChannel(channelNames[i]);
    }
    if (channel->hasClient(clientID)) {
      session->enqueueMsg(
          Response::build("443", nick + " " + channel->getChannelName(),
                          ":is already in channel"));
      return IRC::ERR_USERONCHANNEL;
    }
    result = channel->join(clientID, keys[i]);
    switch (result) {
      case IRC::ERR_BADCHANNELKEY:
        session->enqueueMsg(Response::error(
            "475", nick, channelNames[i] + " :Cannot join channel (+k)"));
        break;
      case IRC::ERR_INVITEONLYCHAN:
        session->enqueueMsg(Response::error(
            "473", nick, channelNames[i] + " :Cannot join channel (+i)"));
        break;
      case IRC::ERR_CHANNELISFULL:
        session->enqueueMsg(Response::error(
            "471", nick, channelNames[i] + " :Cannot join channel (+l)"));
        break;
      case IRC::RPL_NOTOPIC:
      case IRC::RPL_TOPIC: {
        // Send topic and names
        const std::string joinMsg =
            ":" + nick + " JOIN :" + channelNames[i] + "\r\n";
        channel->broadcast(joinMsg, ClientID(-1));
        const std::string& topic = channel->getTopic();
        if (!topic.empty()) {
          session->enqueueMsg(
              Response::build("332", nick, channelNames[i] + " :" + topic));
        } else {
          session->enqueueMsg(Response::build(
              "331", nick, channelNames[i] + " :No topic is set"));
        }
        CommandUtility::sendChannelNames(*session, nick, *channel);
      } break;
      default:
        std::cerr << result << '\n';
        assert(0 && "Unexpected result");
        break;
    }
    lastResult = result;
  }
  return lastResult;
}
