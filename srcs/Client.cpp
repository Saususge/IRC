#include "Client.hpp"

#include <cassert>

#include "numeric.hpp"

Client::Client(ClientID id)
    : _nickname(""),
      _username(""),
      _realname(""),
      _loginFlags(0),
      _registered(false),
      _sessionID(-1),
      _clientID(id) {}

Client::~Client() {}

IRC::Numeric Client::Authenticate() {
  _loginFlags |= IClient::RPASS;
  return IRC::DO_NOTHING;
}

// NICK
// The registry should check for duplicate nicknames before calling this
// function.
IRC::Numeric Client::setNick(const std::string& nick) {
  if (isAuthenticated() == false) {
    return IRC::ERR_REGISTERBEFOREPASS;
  }
  this->_nickname = nick;
  _loginFlags |= IClient::RNICK;
  return checkLoginFlags();
}

// USER
IRC::Numeric Client::setUserInfo(const std::string& user,
                                 const std::string& realName) {
  if (isAuthenticated() == false) {
    return IRC::ERR_REGISTERBEFOREPASS;
  }
  if (_registered) {
    return IRC::ERR_ALREADYREGISTRED;
  }
  _username = user;
  _realname = realName;
  if (!_registered) _loginFlags |= IClient::RUSER;
  return checkLoginFlags();
}

const std::string& Client::getNick() const { return _nickname; }
const std::string& Client::getUser() const { return _username; }
const std::string& Client::getRealName() const { return _realname; }

void Client::setSessionID(SessionID id) { _sessionID = id; }
SessionID Client::getSessionID() const { return _sessionID; }

ClientID Client::getID() const { return _clientID; }

void Client::Register() {
  if (_registered) {
    assert(0 && "Register twice");
  }
  _registered = true;
};

bool Client::isAuthenticated() const { return _loginFlags & IClient::RPASS; }
bool Client::isRegistered() const { return _registered; }

// IRC::Numeric Client::joinChannel(IChannelRegistry& registry,
//                                  const std::string& channelName,
//                                  IClientRegistry& clientRegistry,
//                                  const std::string& key) {
//   if (findFromJoinedChannel(channelName) != _joinedChannels.end())
//     return IRC::DO_NOTHING;

//   IRC::Numeric reply =
//       registry.joinChannel(channelName, _nickname, clientRegistry, key);
//   if (reply <= 400) _joinedChannels.push_back(channelName);
//   return reply;
// }

// IRC::Numeric Client::partChannel(IChannelRegistry& registry,
//                                  const std::string& channelName) {
//   IRC::Numeric reply = registry.partChannel(channelName, _nickname);
//   // partChannel() returned NOSUCHCHANNEL or NOTONCHANNEL but _joinedChannel
//   // contains channelName, it could be a sync error between channel and
//   // client object.
//   if (reply <= 400)
//   _joinedChannels.erase(findFromJoinedChannel(channelName)); return reply;
// }

// const std::vector<std::string>& Client::getJoinedChannels() {
//   return _joinedChannels;
// }

IRC::Numeric Client::checkLoginFlags() {
  if (_loginFlags == (IClient::RPASS | IClient::RNICK | IClient::RUSER)) {
    return IRC::RPL_WELCOME;
  }
  // caller should check that the client has joined any channel. if so,
  // broadcast to all joined channels
  if (_registered) return IRC::RPL_STRREPLY;
  return IRC::DO_NOTHING;
}

// std::vector<std::string>::iterator Client::findFromJoinedChannel(
//     const std::string& channelName) {
//   std::vector<std::string>::iterator iter;
//   for (iter = _joinedChannels.begin(); iter != _joinedChannels.end(); iter++)
//   {
//     if (*iter == channelName) break;
//   }
//   return iter;
// }