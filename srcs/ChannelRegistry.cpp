#include "ChannelRegistry.hpp"

#include <map>
#include <string>
#include <utility>

#include "Channel.hpp"
#include "IChannel.hpp"

ChannelRegistry::~ChannelRegistry() {
  for (std::map<std::string, IChannel*>::iterator it = _channels.begin();
       it != _channels.end(); ++it)
    delete it->second;
};

std::string ChannelRegistry::createChannel(const std::string& channelName) {
  std::map<std::string, IChannel*>::iterator it = _channels.find(channelName);
  if (it != _channels.end()) {
    delete it->second;
  }
  _channels[channelName] = new Channel(channelName);
  return channelName;
}
void ChannelRegistry::deleteChannel(const std::string& channelName) {
  std::map<std::string, IChannel*>::iterator it = _channels.find(channelName);
  if (it == _channels.end()) {
    return;
  }
  delete it->second;
  _channels.erase(it);
}

IChannel* ChannelRegistry::getChannel(const std::string& channelName) {
  std::map<std::string, IChannel*>::iterator it = _channels.find(channelName);
  if (it == _channels.end()) {
    return NULL;
  }
  return it->second;
}

std::set<const IChannel*> ChannelRegistry::getChannels() {
  std::set<const IChannel*> ret;
  for (std::map<std::string, IChannel*>::iterator it = _channels.begin();
       it != _channels.end(); ++it) {
    ret.insert(it->second);
  }
  return ret;
}
