#ifndef CHANNELREGISTRY_HPP
#define CHANNELREGISTRY_HPP

#include <map>

#include "IChannel.hpp"
#include "IChannelRegistry.hpp"

class ChannelRegistry : public IChannelRegistry {
 public:
  ~ChannelRegistry();

  std::string createChannel(const std::string& channelName);
  void deleteChannel(const std::string& id);

  IChannel* getChannel(const std::string& id);
  const std::set<IChannel*> getChannels();

 private:
  std::map<std::string, IChannel*> _channels;
};

#endif  // CHANNELREGISTRY_HPP