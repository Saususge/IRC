#include <cassert>
#include <cstdlib>
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

IRC::Numeric ChannelModeCommand::execute(ICommandContext& ctx) const {
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
    session->enqueueMsg(
        Response::error("461", nick, "MODE :Not enough parameters"));
    return IRC::ERR_NEEDMOREPARAMS;
  }

  const std::string& channelName = ctx.args()[0];
  if (!Validator::isChannelNameValid(channelName)) {
    session->enqueueMsg(
        Response::error("502", nick, ":Can't change mode for other users"));
    return IRC::ERR_USERSDONTMATCH;
  }

  IChannel* channel = ChannelManagement::getChannel(channelName);
  if (channel == NULL) {
    session->enqueueMsg(
        Response::error("403", nick, channelName + " :No such channel"));
    return IRC::ERR_NOSUCHCHANNEL;
  }
  if (channelName[0] == '+') {
    session->enqueueMsg(
        Response::error(IRC::ERR_NOCHANMODES, nick,
                        channelName + " :Channel doesn't support modes"));
    return IRC::ERR_NOCHANMODES;
  }

  if (ctx.args().size() == 1) {
    session->enqueueMsg(Response::build(
        "324", nick,
        channelName + " " + CommandUtility::getFullModeResponse(channel)));
    return IRC::RPL_CHANNELMODEIS;
  }

  std::vector<std::string> rawParams;
  for (size_t i = 2; i < ctx.args().size(); ++i)
    rawParams.push_back(ctx.args()[i]);

  std::vector<std::pair<bool, char> > modePairs;
  std::vector<std::string> params;
  CommandUtility::parseModeArguments(ctx.args()[1], rawParams, modePairs,
                                     params);

  std::string appliedModes = "";
  std::string appliedParams = "";
  bool currentAdding = true;
  bool firstSuccess = true;
  size_t usedParamIdx = 0;
  bool hasChanged = false;

  for (size_t i = 0; i < modePairs.size(); ++i) {
    bool isAdding = modePairs[i].first;
    char mode = modePairs[i].second;
    IRC::Numeric result = IRC::DO_NOTHING;
    bool needParam =
        (mode == 'o' || (mode == 'k' && isAdding) || (mode == 'l' && isAdding));
    std::string currentParam = needParam ? params[usedParamIdx] : "";

    switch (mode) {
      case 'i':
        result = isAdding ? channel->setInviteOnly(clientID)
                          : channel->unsetInviteOnly(clientID);
        break;
      case 't':
        result = isAdding ? channel->setTopicOpOnly(clientID)
                          : channel->unsetTopicOpOnly(clientID);
        break;
      case 'k':
        result = isAdding ? channel->setKey(clientID, currentParam)
                          : channel->unsetKey(clientID);
        break;
      case 'l':
        result = isAdding ? channel->setLimit(clientID,
                                              std::atoi(currentParam.c_str()))
                          : channel->unsetLimit(clientID);
        break;
      case 'o': {
        IClient* targetClient = ClientManagement::getClient(currentParam);
        if (targetClient == NULL ||
            !channel->hasClient(targetClient->getID())) {
          session->enqueueMsg(
              Response::error("441", nick,
                              currentParam + " " + channelName +
                                  " :They aren't on that channel"));
          result = IRC::ERR_USERNOTINCHANNEL;
        } else {
          result =
              isAdding
                  ? channel->setClientOp(clientID, targetClient->getID())
                  : channel->unsetClientOp(clientID, targetClient->getID());
        }
      } break;
      default:
        session->enqueueMsg(Response::error(
            "472", nick, std::string(1, mode) + " :is unknown mode char"));
        continue;
    }
    if (result == IRC::RPL_STRREPLY) {
      hasChanged = true;
      if (firstSuccess || currentAdding != isAdding) {
        appliedModes += (isAdding ? '+' : '-');
        currentAdding = isAdding;
        firstSuccess = false;
      }
      appliedModes += mode;
      if (!currentParam.empty()) {
        if (!appliedParams.empty()) appliedParams += " ";
        appliedParams += currentParam;
      }
    } else if (result == IRC::ERR_CHANOPRIVSNEEDED) {
      session->enqueueMsg(Response::error(
          "482", nick, channelName + " :You're not channel operator"));
      break;  // Early stop
    } else if (result == IRC::ERR_NOCHANMODES) {
      session->enqueueMsg(Response::error(
          "477", nick, channelName + " :Channel doesn't support modes"));
      break;  // Early stop
    } else if (result == IRC::ERR_KEYSET) {
      session->enqueueMsg(Response::error(
          "467", nick, channelName + " :Channel key already set"));
    } else if (result == IRC::ERR_NEEDMOREPARAMS) {
      session->enqueueMsg(
          Response::error("461", nick, "MODE :Not enough parameters"));
    } else if (result == IRC::ERR_USERNOTINCHANNEL) {
      // already handled above for +o/-o
    } else if (result == IRC::DO_NOTHING) {
      // no change needed (e.g., mode already set/unset)
    } else {
      assert(0 && "Unexpected result");
    }
    if (needParam) {
      ++usedParamIdx;
    }
  }

  if (hasChanged) {
    std::string diffString = appliedModes;
    if (!appliedParams.empty()) diffString += " " + appliedParams;
    std::string modeNotification =
        ":" + nick + " MODE " + channelName + " " + diffString + "\r\n";
    channel->broadcast(modeNotification, ClientID(-1));
    session->enqueueMsg(Response::build(
        "324", nick,
        channelName + " " + CommandUtility::getFullModeResponse(channel)));
  }

  return IRC::RPL_CHANNELMODEIS;
}
