#include "Client.hpp"

Client::Client()
    : _nickname(""), _username(""), _realname(""), loginFlags(0), registerd(false), _refCount(0) {}

Client::Client(std::string nickname, std::string username, std::string realname)
    : _nickname(nickname), _username(username), _realname(realname), loginFlags(0), registerd(false), _refCount(0) {}

Client::~Client() {}

std::string Client::getNickname() const { return _nickname; }

void Client::setNickname(std::string nickname) { _nickname = nickname; }

std::string Client::getUsername() const { return _username; }

std::string Client::getRealname() const { return _realname; }


void Client::onPass() { loginFlags |= 0b001; }
void Client::onNick() { loginFlags |= 0b010; }
void Client::onUser() { loginFlags |= 0b100; }

bool Client::getPass() const { return loginFlags & 0b001; }
bool Client::getNick() const { return loginFlags & 0b010; }
bool Client::getUser() const { return loginFlags & 0b100; }
short Client::getLoginFlags() const { return loginFlags; }

bool Client::isRegistrable() const { return loginFlags == 0b111; }

void Client::setRegisterd(bool value) { registerd = value; }
bool Client::getRegisterd() const { return registerd; }

void Client::joinChannel(std::string channelName) {
  _joinedChannels.push_back(channelName);
}

// TODO: leaveChannel 구현
void Client::leaveChannel(std::string channelName) {
  (void)channelName;
}

void Client::addRef() {++_refCount;}
void Client::release() {
  if (_refCount == 0) {
    delete this;
    return;
  }
  --_refCount;
}
