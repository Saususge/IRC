#include "Client.hpp"

#include <string>

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

IRC::Numeric Client::joinChannel(const std::string& channelName) {
  if (channelName == "0") {
    _joinedChannels.clear();
  } else {
    _joinedChannels.insert(channelName);
  }
  return IRC::DO_NOTHING;
}

IRC::Numeric Client::partChannel(const std::string& channelName) {
  _joinedChannels.erase(channelName);
  return IRC::DO_NOTHING;
}
