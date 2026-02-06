#ifndef CHANNELREGISTRY_HPP
#define CHANNELREGISTRY_HPP

#include <map>

#include "IChannel.hpp"
#include "IChannelRegistry.hpp"

class ChannelRegistry : public IChannelRegistry {
 public:
  ~ChannelRegistry();

  std::string createChannel(const std::string& channelName);
  void deleteChannel(const std::string& channelName);

  IChannel* getChannel(const std::string& channelName);
  std::set<IChannel*> getChannels();

 private:
  std::map<std::string, IChannel*> _channels;
};

#endif  // CHANNELREGISTRY_HPP