#ifndef ICHANNELREGISTRY_HPP
#define ICHANNELREGISTRY_HPP

#include <set>

#include "IChannel.hpp"

class IChannelRegistry {
 public:
  virtual ~IChannelRegistry() {};

  virtual std::string createChannel(const std::string& channelName) = 0;
  virtual void deleteChannel(const std::string& channelName) = 0;

  virtual IChannel* getChannel(const std::string& channelName) = 0;
  virtual std::set<IChannel*> getChannels() = 0;
};
#endif  // ICHANNELREGISTRY_HPP