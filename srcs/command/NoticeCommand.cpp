#include <string>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "Command.hpp"
#include "CommandUtility.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ICommand.hpp"
#include "SessionManagement.hpp"
#include "numeric.hpp"

// Numeric Replies: Same as PRIVMSG except no automatic replies
IRC::Numeric NoticeCommand::execute(ICommandContext& ctx) const {
  ClientID clientID = ctx.clientID();
  // NOTICE never returns errors
  if (ctx.args().empty() || ctx.args().size() < 2 || ctx.args()[1].empty()) {
    return IRC::DO_NOTHING;
  }
  const std::string& target = ctx.args()[0];
  const std::string& message = ctx.args()[1];
  const std::string prefix = CommandUtility::getClientPrefix(clientID);
  const std::string noticeNotification =
      ":" + prefix + " NOTICE " + target + " :" + message + "\r\n";

  if (target[0] == '#' || target[0] == '&' || target[0] == '+') {
    // Channel notice
    IChannel* channel = ChannelManagement::getChannel(target);
    if (channel == NULL) {
      return IRC::DO_NOTHING;
    }
    channel->broadcast(noticeNotification, clientID);
    return IRC::DO_NOTHING;
  }

  // Private notice to user
  IClient* targetClient = ClientManagement::getClient(target);
  if (targetClient == NULL) {
    return IRC::DO_NOTHING;
  }
  SessionManagement::getSession(targetClient)->enqueueMsg(noticeNotification);
  return IRC::DO_NOTHING;
}
