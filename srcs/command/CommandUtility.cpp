#include "CommandUtility.hpp"

#include <cassert>
#include <cstdlib>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "IChannel.hpp"
#include "IClient.hpp"
#include "ISession.hpp"
#include "Response.hpp"
#include "SessionManagement.hpp"
#include "defs.hpp"

namespace CommandUtility {

void sendWelcomeMessageAndRegister(ICommandContext& ctx) {
  ClientID clientID = ctx.clientID();
  SessionID sessionID = ctx.sessionID();
  IClient* client = ClientManagement::getClient(clientID);
  ISession* session = SessionManagement::getSession(sessionID);

  if (client == NULL || session == NULL) {
    return;
  }

  const std::string& nick = client->getNick().empty() ? "*" : client->getNick();
  session->enqueueMsg(Response::build(
      "001", nick, ":Welcome to the Internet Relay Network " + nick));
  session->enqueueMsg(Response::build(
      "002", nick,
      ":Your host is " + ctx.serverConfig().getServerName() +
          ", running version " + ctx.serverConfig().getVersion()));
  session->enqueueMsg(Response::build(
      "003", nick,
      ":This server was created " + ctx.serverConfig().getCreationDate()));
  session->enqueueMsg(
      Response::build("004", nick,
                      ctx.serverConfig().getServerName() + " " +
                          ctx.serverConfig().getVersion() + " " +
                          ctx.serverConfig().getUserModes() + " " +
                          ctx.serverConfig().getChannelModes()));
  client->Register();
}

std::set<IChannel*> getJoinedChannels(ClientID client) {
  std::set<IChannel*> _joinedChannels = ChannelManagement::getChannels();
  std::set<IChannel*>::iterator it = _joinedChannels.begin();
  while (it != _joinedChannels.end()) {
    if (!(*it)->hasClient(client)) {
      _joinedChannels.erase(it++);
    } else {
      ++it;
    }
  }
  return _joinedChannels;
}

void sendChannelNames(ISession& requester, const std::string& requesterNick,
                      IChannel& channel) {
  const std::string channelName = channel.getChannelName();
  const std::set<ClientID> joinedClients = channel.getJoinedClients();

  std::string namesList;
  namesList.reserve(512);

  for (std::set<ClientID>::const_iterator it = joinedClients.begin();
       it != joinedClients.end(); ++it) {
    std::string nickPrefix = channel.isClientOp(*it) ? "@" : "";
    std::string clientNick = ClientManagement::getClient(*it)->getNick();
    if (!namesList.empty()) namesList += " ";
    namesList += nickPrefix + clientNick;
  }
  if (!namesList.empty()) {
    requester.enqueueMsg(Response::build(
        "353", requesterNick, "= " + channelName + " :" + namesList));
  }
  requester.enqueueMsg(Response::build("366", requesterNick,
                                       channelName + " :End of NAMES list"));
}

void sendWildcardNames(ISession& requester, const std::string& requesterNick,
                       const std::set<ClientID>& remainClientIDs) {
  if (remainClientIDs.empty()) {
    return;
  }

  std::string namesList;
  namesList.reserve(512);
  for (std::set<ClientID>::const_iterator it = remainClientIDs.begin();
       it != remainClientIDs.end(); ++it) {
    std::string clientNick = ClientManagement::getClient(*it)->getNick();
    if (!namesList.empty()) {
      namesList += " ";
    }
    namesList += clientNick;
  }
  if (!namesList.empty()) {
    requester.enqueueMsg(
        Response::build("353", requesterNick, "= * :" + namesList));
  }
  requester.enqueueMsg(
      Response::build("366", requesterNick, "* :End of NAMES list"));
}

std::vector<std::string> split(const std::string& str, const std::string& sep) {
  std::vector<std::string> result;
  if (sep.empty()) {
    result.push_back(str);
    return result;
  }
  std::string::size_type start = 0;
  std::string::size_type end = str.find(sep);
  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + sep.length();
    end = str.find(sep, start);
  }
  result.push_back(str.substr(start));
  return result;
}

void parseModeArguments(const std::string& modeStr,
                        const std::vector<std::string>& rawParams,
                        std::vector<std::pair<bool, char> >& modePairs,
                        std::vector<std::string>& params) {
  bool isAdding = true;
  size_t paramIdx = 0;
  int paramModeCount = 0;

  for (size_t i = 0; i < modeStr.size(); ++i) {
    char c = modeStr[i];
    if (c == '+') {
      isAdding = true;
      continue;
    }
    if (c == '-') {
      isAdding = false;
      continue;
    }
    bool needsParam =
        (c == 'o' || (c == 'k' && isAdding) || (c == 'l' && isAdding));

    if (needsParam) {
      if (paramModeCount >= 3) continue;

      if (paramIdx < rawParams.size()) {
        modePairs.push_back(std::make_pair(isAdding, c));
        params.push_back(rawParams[paramIdx++]);
        paramModeCount++;
      }
    } else {
      modePairs.push_back(std::make_pair(isAdding, c));
    }
  }
}

std::string intToString(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

std::string getFullModeResponse(IChannel* channel) {
  std::string modes = "+";
  std::string params = "";

  if (channel->isInviteOnly()) modes += 'i';
  if (channel->isTopicOpOnly()) modes += 't';
  if (channel->hasKey()) modes += 'k';
  if (channel->isLimited()) {
    modes += 'l';
    params += " " + intToString(channel->getMaxMember());
  }
  return modes + params;
}

}  // namespace CommandUtility
