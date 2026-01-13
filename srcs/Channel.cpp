#include "Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}

void Channel::addUser(std::string nickname) {}
void Channel::delUser(std::string nickname) {}
std::unordered_map<std::string, MemberInfo> Channel::getUsers() const {}

int Channel::promoteToOp(std::string prompter, std::string target) {}

void Channel::setInvite(bool value) {}
bool Channel::getInvite() const {}
void Channel::inviteUser(std::string nickname) {}

void Channel::setTopicMode(bool value) {}
bool Channel::getTopicMode() const {}
void Channel::setTopic(std::string topic) {}
std::string Channel::getTopic() const {}

void Channel::setKey(std::string newKey) {}
std::string Channel::getKey() const {}