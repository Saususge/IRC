#ifndef CHANNELMANAGEMENT_HPP
#define CHANNELMANAGEMENT_HPP

#include <string>

#include "IChannel.hpp"

namespace ChannelManagement {
std::string createChannel(const std::string& channelName);
void deleteChannel(const std::string& channelName);

IChannel* getChannel(const std::string& channelName);
std::set<IChannel*> getChannels();
};  // namespace ChannelManagement

#endif