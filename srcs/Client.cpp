#include "Client.hpp"

Client::Client(std::string nickname, std::string username, std::string realname)
    : _nickname(nickname), _username(username), _realname(realname) {}

Client::~Client() {}

std::string Client::getNickname() const { return _nickname; }

void Client::setNickname(std::string nickname) { _nickname = nickname; }

std::string Client::getUsername() const { return _username; }

std::string Client::getRealname() const { return _realname; }

void Client::joinChannel(std::string channelName) {
  _joinedChannels.push_back(channelName);
}

// TODO: leaveChannel 구현
void Client::leaveChannel(std::string channelName) {
  (void)channelName;
}
