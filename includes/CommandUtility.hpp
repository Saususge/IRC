#ifndef COMMANDUTILITY_HPP
#define COMMANDUTILITY_HPP

#include <set>
#include <string>
#include <vector>

#include "IChannel.hpp"
#include "ICommand.hpp"
#include "ISession.hpp"
#include "defs.hpp"

namespace CommandUtility {

void sendWelcomeMessage(ICommandContext& ctx);

std::set<IChannel*> getJoinedChannels(ClientID client);

void sendChannelNames(ISession& requester, const std::string& requesterNick,
                      IChannel& channel);

void sendWildcardNames(ISession& requester, const std::string& requesterNick,
                       const std::set<ClientID>& remainClientIDs);

std::vector<std::string> split(const std::string& str,
                               const std::string& sep = ",");

void parseModeArguments(const std::string& modeStr,
                        const std::vector<std::string>& rawParams,
                        std::vector<std::pair<bool, char> >& modePairs,
                        std::vector<std::string>& params);

std::string intToString(int n);

std::string getFullModeResponse(IChannel* channel);

}  // namespace CommandUtility

#endif
