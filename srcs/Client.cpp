#include "Client.hpp"

#include <string>

#include "IChannelRegistry.hpp"
#include "Validator.hpp"
#include "numeric.hpp"

// Pass
IRC::Numeric Client::Authenticate(IServerConfig& serverConfig,
                                  const std::string& password) {
  if (password.empty()) {
    return IRC::ERR_NEEDMOREPARAMS;
  }
  if (isAuthenticated()) {
    return IRC::ERR_ALREADYREGISTRED;
  }
  if (serverConfig.getPassword() == password) {
    _loginFlags |= 0b001;
  }
  return IRC::DO_NOTHING;
}
// NICK
IRC::Numeric Client::setNick(IClientRegistry& registry,
                             const std::string& nick) {
  if (nick.empty()) {
    return IRC::ERR_NONICKNAMEGIVEN;
  }
  if (!Validator::isNickValid(nick)) {
    return IRC::ERR_ERRONEUSNICKNAME;
  }
  if (registry.isNickInUse(nick)) {
    return IRC::ERR_NICKNAMEINUSE;
  }
  // Omit ERR_NICKCOLLISION, ERR_UNAVAILRESOURCE, ERR_RESTRICTED
  _nickname = nick;
  _loginFlags |= 0b010;
  return IRC::DO_NOTHING;
}
// USER
IRC::Numeric Client::setUserInfo(const std::string& user,
                                 const std::string& realName) {
  if (user.empty() || realName.empty()) {
    return IRC::ERR_NEEDMOREPARAMS;
  }
  if (_loginFlags & 0b100) {
    return IRC::ERR_ALREADYREGISTRED;
  }
  return IRC::DO_NOTHING;
}

IRC::Numeric Client::joinChannel(IChannelRegistry& registry,
                                 const std::string& channelName) {
  IRC::Numeric ret = registry.joinChannel(channelName, _nickname);
  if (ret == IRC::DO_NOTHING) {
    // Success
    _joinedChannels.insert(channelName);
  }
  return ret;
}

IRC::Numeric Client::partChannel(IChannelRegistry& registry,
                                 const std::string& channelName) {
  IRC::Numeric ret = registry.partChannel(channelName, _nickname);
  if (ret == IRC::DO_NOTHING) {
    // Success
    _joinedChannels.erase(channelName);
  }
  return ret;
}
