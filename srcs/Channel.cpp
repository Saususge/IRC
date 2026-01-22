#include "Channel.hpp"

Channel::Channel() {
  inviteOnly = false;
  restrictTopic = false;
  topic = "";
  key = "";
}

Channel::~Channel() {}

int Channel::addUser(int fd, Client* client, bool isCreator) {
  if (getUserInfo(client->getNickname()) == 1) {
#ifdef DEBUG
    std::cerr << "User " << client->getNickname() << " already exists." << std::endl;
#endif
    return -1;
  }
  users.insert(std::pair<std::string, MemberInfo>(client->getNickname(), MemberInfo(fd, client, isCreator)));
#ifdef DEBUG
    std::cerr << "User " << client->getNickname() << " joined to the channel." << std::endl;
#endif
  return 1;
}

int Channel::delUser(Client* client) {
  if (getUserInfo(client->getNickname()) == -1) return -1;

  users.erase(client->getNickname());
  return 1;
}

int Channel::getUserInfo(std::string nickname, MemberInfo* info) {
  mIter memberIter = users.find(nickname);
  if (memberIter == users.end()) {
#ifdef DEBUG
    std::cerr << "User " << nickname << " not found in channel." << std::endl;
#endif
    return -1;
  }

  if (info != NULL) *info = memberIter->second;
  return 1;
}

const std::map<std::string, MemberInfo>& Channel::getUsers() const {
  return users;
}

int Channel::promoteToOp(Client* client, std::string target) {
  MemberInfo info;
  if (getUserInfo(client->getNickname(), &info) == -1) return -1;

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "User " << client->getNickname() << " is not channel operator."
              << std::endl;
#endif
    return -2;
  }

  if (getUserInfo(target, &info) == -1) return -1;

  info.op = true;
  return 1;
}

int Channel::setInvite(Client* client, bool value) {
  (void)client;                         // TODO: check fd is operator
  if (value == inviteOnly) return 0;  // do nothing

  inviteOnly = value;
  return 1;
}
bool Channel::getInvite() const { return inviteOnly; }

int Channel::inviteUser(Client* client, std::string target) {
  MemberInfo info;

  (void)client;  // TODO: check fd is operator
  if (getUserInfo(target, &info) == -1) return -1;

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "User " << client->getNickname() << " is not operator." << std::endl;
#endif
    return -2;
  }
  invitedUsers.insert(target);
  return 1;
}

void Channel::setTopicMode(Client* client, bool value) {  // not implemented
  (void)client;
  (void)value;
}

bool Channel::getTopicMode() const { return restrictTopic; }

void Channel::setTopic(Client* client, std::string topic) {  // not implemented
  (void)client;
  (void)topic;
}

std::string Channel::getTopic() const { return topic; }

void Channel::setKey(Client* client, std::string newKey) {  // not implemented
  (void)client;
  (void)newKey;
}

std::string Channel::getKey() const { return key; }