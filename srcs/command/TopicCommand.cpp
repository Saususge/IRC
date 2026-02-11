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
// ERR_CHANOPRIVSNEEDED, RPL_NOTOPIC, RPL_TOPIC
IRC::Numeric TopicCommand::execute(ICommandContext& ctx) const {
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
    // ERR_NEEDMOREPARAMS (461)
    session->enqueueMsg(
        Response::error("461", nick, "TOPIC :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }
  const std::string& channelName = ctx.args()[0];
  IChannel* channel = ChannelManagement::getChannel(channelName);
  if (channel == NULL) {
    // ERR_NOSUCHCHANNEL (403)
    session->enqueueMsg(
        Response::error("403", nick, channelName + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }
  if (!channel->hasClient(clientID)) {
    // ERR_NOTONCHANNEL (442)
    session->enqueueMsg(Response::error(
        "442", nick, channelName + " :You're not on that channel"));
    return IRC::ERR_NOTONCHANNEL;
  }

  // Send topic
  if (ctx.args().size() == 1) {
    const std::string& topic = channel->getTopic();
    if (topic.empty()) {
      // RPL_NOTOPIC (331)
      session->enqueueMsg(
          Response::build("331", nick, channelName + " :No topic is set"));
      return IRC::RPL_NOTOPIC;
    }
    // RPL_TOPIC (332)
    session->enqueueMsg(
        Response::build("332", nick, channelName + " :" + topic));
    return IRC::RPL_TOPIC;
  }

  // Set topic
  const std::string& newTopic = ctx.args()[1];
  IRC::Numeric result = channel->setTopic(clientID, newTopic);
  switch (result) {
    case IRC::ERR_CHANOPRIVSNEEDED:
      session->enqueueMsg(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;
    case IRC::RPL_STRREPLY: {
      // Topic changed successfully - broadcast to channel
      const std::string topicMsg =
          ":" + nick + " TOPIC " + channelName + " :" + newTopic + "\r\n";
      channel->broadcast(topicMsg, ClientID(-1));
    } break;
    default:
      assert(0 && "Unexpected result");
      break;
  }
  return result;
}
