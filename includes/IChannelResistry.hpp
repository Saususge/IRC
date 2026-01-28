#ifndef ICHANNELRESISTRY_HPP
#define ICHANNELRESISTRY_HPP

#include <string>
#include <vector>

class IChannelResistry {
 public:
  virtual ~IChannelResistry() {};

  virtual int createChannel(const std::string ChannelName) = 0;
  virtual int deleteChannel(const std::string& channelName) = 0;
  virtual bool hasChannel(const std::string& channelName) = 0;
  virtual const std::vector<const std::string>& getChannels() = 0;

  // Use `except` only if excluding sender.
  virtual int broadcast(const std::string& channelName, const std::string& msg,
                        const std::string& except = "") = 0;
};
#endif  // ICHANNELRESISTRY_HPP