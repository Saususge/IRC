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
  _channelReg.deleteChannel(channelName);
}

IChannel* getChannel(const std::string& channelName) {
  return _channelReg.getChannel(channelName);
}
std::set<IChannel*> getChannels() {
  std::set<const IChannel*> constChannels = _channelReg.getChannels();
  std::set<IChannel*> result;
  for (std::set<const IChannel*>::iterator it = constChannels.begin();
       it != constChannels.end(); ++it) {
    result.insert(const_cast<IChannel*>(*it));
  }
  return result;
}
};  // namespace ChannelManagement
