#include "Client.hpp"

#include "Session.hpp"

Client::Client(ISession& session, IServerConfig& serverConfig)
    : _nickname(""),
      _username(""),
      _realname(""),
      loginFlags(0),
      registered(false),
      session(session) {
  if (serverConfig.getPassword().empty()) loginFlags |= IClient::RPASS;
}

Client::~Client() {}

IRC::Numeric Client::Authenticate(const IServerConfig& serverConfig,
                                  const std::string& password) {
  if (registered) return IRC::ERR_ALREADYREGISTRED;

  if (serverConfig.getPassword() == password) loginFlags |= IClient::RPASS;
  return IRC::DO_NOTHING;
}

// NICK
// The registry should check for duplicate nicknames before calling this
// function.
IRC::Numeric Client::setNick(IClientRegistry& registry,
                             const std::string& nick) {
  this->_nickname = nick;
  if (!registered) loginFlags |= IClient::RNICK;
  return checkLoginFlags();
}

// USER
IRC::Numeric Client::setUserInfo(const std::string& user,
                                 const std::string& realName) {
  if (registered) return IRC::ERR_ALREADYREGISTRED;
  _username = user;
  _realname = realName;
  if (!registered) loginFlags |= IClient::RUSER;
  return checkLoginFlags();
}

const std::string& Client::getNick() { return _nickname; }
const std::string& Client::getUser() { return _username; }
const std::string& Client::getRealName() { return _realname; }

// ClientResistry or equivalent has to send.
int Client::send(const std::string& msg) { return session.send(msg); }

IRC::Numeric Client::joinChannel(IChannelRegistry& registry,
                                 const std::string& channelName,
                                 const std::string& key) {
  // JOIN does not return NOTONCHANNEL
  if (findFromJoinedChannel(channelName) != _joinedChannels.end())
    return IRC::DO_NOTHING;

  IRC::Numeric reply = registry.joinChannel(channelName, _nickname, key);
  if (reply <= 400) _joinedChannels.push_back(channelName);
  return reply;
}

IRC::Numeric Client::partChannel(IChannelRegistry& registry,
                                 const std::string& channelName) {
  IRC::Numeric reply = registry.partChannel(channelName, _nickname);
  // partChannel() returned NOSUCHCHANNEL or NOTONCHANNEL but _joinedChannel
  // contains channelName, it could be a sync error between channel and
  // client object.
  if (reply <= 400) _joinedChannels.erase(findFromJoinedChannel(channelName));
  return reply;
}

const std::vector<std::string>& Client::getJoinedChannels() {
  return _joinedChannels;
}

IRC::Numeric Client::checkLoginFlags() {
  if (loginFlags == (IClient::RPASS | IClient::RNICK | IClient::RUSER)) {
    registered = true;
    return IRC::RPL_WELCOME;
  } else if (loginFlags == (IClient::RNICK | IClient::RUSER)) {
    return IRC::ERR_PASSWDMISMATCH;
  }
  // caller should check that the client has joined any channel. if so,
  // broadcast to all joined channels
  return IRC::RPL_STRREPLY;
}

std::vector<std::string>::iterator Client::findFromJoinedChannel(
    const std::string& channelName) {
  std::vector<std::string>::iterator iter;
  for (iter = _joinedChannels.begin(); iter != _joinedChannels.end(); iter++) {
    if (*iter == channelName) break;
  }
  return iter;
}