#include "ChannelRegistry.hpp"

// ChannelRegistry singleton
namespace {
ChannelRegistry _channelReg;
}

namespace ChannelManagement {
std::string createChannel(const std::string& channelName) {
  return _channelReg.createChannel(channelName);
}
void deleteChannel(const std::string& channelName) {
  _channelReg.createChannel(channelName);
}

IChannel* getChannel(const std::string& channelName) {
  return _channelReg.getChannel(channelName);
}
std::set<const IChannel*> getChannels() { return _channelReg.getChannels(); }
};  // namespace ChannelManagement
