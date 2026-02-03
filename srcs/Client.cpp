#include "Client.hpp"

// TODO: initialize session object
Client::Client(IServerConfig& serverConfig)
    : _nickname(""),
      _username(""),
      _realname(""),
      loginFlags(0),
      registerd(false),
      session(NULL) {
  if (serverConfig.getPassword().empty()) loginFlags |= IClient::RPASS;
}

Client::~Client() {}

IRC::Numeric Client::Authenticate(IServerConfig& serverConfig,
                                  const std::string& password) {
  if (registerd) return IRC::ERR_ALREADYREGISTRED;

  if (serverConfig.getPassword() == password) loginFlags |= IClient::RPASS;
  return IRC::DO_NOTHING;
}

// NICK
IRC::Numeric Client::setNick(IClientRegistry& registry,
                             const std::string& nick) {
  if (registry.isNickInUse(nick)) return IRC::ERR_NICKNAMEINUSE;
  // TODO: validate nickname if not, return ERR_ERRONEUSNICKNAME
  this->_nickname = nick;
  loginFlags |= IClient::RNICK;
  if (loginFlags == 0b111) {
    registerd = true;
    return IRC::RPL_WELCOME;
  }
  // caller should check that the client has joined any channel. if so,
  // broadcast to all joined channels
  return IRC::RPL_STRREPLY;
}

// USER
IRC::Numeric Client::setUserInfo(const std::string& user,
                                 const std::string& realName) {
  if (registerd) return IRC::ERR_ALREADYREGISTRED;
  _username = user;
  _realname = realName;
  loginFlags |= IClient::RUSER;
  if (loginFlags == 0b111) {
    registerd = true;
    return IRC::RPL_WELCOME;
  }
  // caller should check that the client has joined any channel. if so,
  // broadcast to all joined channels
  return IRC::RPL_STRREPLY;
}

const std::string& Client::getNick() { return _nickname; }
const std::string& Client::getUser() { return _username; }
const std::string& Client::getRealName() { return _realname; }

// ClientResistry or equivalent has to send.
// int Client::send(const std::string& msg) { return session->send(msg); }

IRC::Numeric Client::joinChannel(IChannelRegistry& registry,
                                 const std::string& channelName,
                                 const std::string& key) {
  IRC::Numeric reply = registry.joinChannel(channelName, _nickname, key);
  if (reply <= 400) _joinedChannels.push_back(channelName);
  return reply;
}
IRC::Numeric Client::partChannel(IChannelRegistry& registry,
                                 const std::string& channelName) {
  IRC::Numeric reply = registry.partChannel(channelName, _nickname);
  if (reply <= 400) {
    for (std::vector<std::string>::iterator iter = _joinedChannels.begin();
         iter != _joinedChannels.end(); iter++) {
      // partChannel() returned NOSUCHCHANNEL or NOTONCHANNEL but _joinedChannel
      // contains channelName, it could be a sync error between channel and
      // client object.
      if (*iter == channelName) {
        _joinedChannels.erase(iter);
        break;
      }
    }
  }
  return reply;
}

const std::vector<std::string>& Client::getJoinedChannels() {
  return _joinedChannels;
}
