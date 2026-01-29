#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <set>
#include <string>

#include "IChannelRegistry.hpp"
#include "IClient.hpp"
#include "numeric.hpp"

class Client : public IClient {
 public:
  Client() : _loginFlags(0) {}
  ~Client() {}

  // Pass
  IRC::Numeric Authenticate(IServerConfig& serverConfig,
                            const std::string& password);
  // NICK
  IRC::Numeric setNick(IClientRegistry& registry, const std::string& nick);
  // USER
  IRC::Numeric setUserInfo(const std::string& user,
                           const std::string& realName);

  inline const std::string& getNick() { return _nickname; }
  inline const std::string& getUser() { return _username; }
  inline const std::string& getRealName() { return _realname; }

  bool isRegisterable() { return _loginFlags & 0b111; }

  IRC::Numeric joinChannel(IChannelRegistry& registry,
                           const std::string& channelName);
  IRC::Numeric partChannel(IChannelRegistry& registry,
                           const std::string& channelName);
  const std::set<std::string>& getJoinedChannels() { return _joinedChannels; }

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;

  // PASS NICK USER <- Least significant bit
  int _loginFlags;

  std::set<std::string> _joinedChannels;

  inline bool isAuthenticated() { return _loginFlags & 0b001; }
};

#endif
