#ifndef ICLIENT_HPP
#define ICLIENT_HPP

#include <string>
#include <vector>

#include "IChannelRegistry.hpp"
#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "numeric.hpp"

class IClient {
 public:
  virtual ~IClient() {};

  // Pass
  virtual IRC::Numeric Authenticate(IServerConfig serverConfig,
                                    const std::string& password) = 0;
  // NICK
  virtual IRC::Numeric setNick(IClientRegistry& registry,
                               const std::string& nick) = 0;
  // USER
  virtual IRC::Numeric setUserInfo(const std::string& user,
                                   const std::string& realName) = 0;
  virtual const std::string& getNick() = 0;
  virtual const std::string& getUser() = 0;
  virtual const std::string& getRealName() = 0;

  // ClientResistry or equivalent has to send.
  // virtual int send(const std::string& msg) = 0;

  virtual IRC::Numeric joinChannel(IChannelRegistry& registry,
                                   const std::string& channelName) = 0;
  virtual IRC::Numeric partChannel(IChannelRegistry& registry,
                                   const std::string& channelName) = 0;
  virtual const std::vector<std::string>& getJoinedChannels() = 0;
};
#endif  // ICLIENT_HPP