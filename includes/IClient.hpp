#ifndef ICLIENT_HPP
#define ICLIENT_HPP

#include <set>
#include <string>

#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "numeric.hpp"

class IClient {
 public:
  virtual ~IClient() {};

  // Pass
  virtual IRC::Numeric Authenticate(IServerConfig& serverConfig,
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

  virtual bool isRegisterable() = 0;

  // ClientResistry or equivalent has to send.
  // virtual int send(const std::string& msg) = 0;

  // Client doesn't know about the existance of the channel
  virtual IRC::Numeric joinChannel(const std::string& channelName) = 0;
  // Client doesn't know about the existance of the channel
  virtual IRC::Numeric partChannel(const std::string& channelName) = 0;
  virtual const std::set<std::string>& getJoinedChannels() = 0;
};
#endif  // ICLIENT_HPP