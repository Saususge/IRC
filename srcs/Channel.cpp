#include "Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}

int Channel::addUser(std::string nickname, int fd, bool isCreator) {
  if (getUserInfo(nickname) == 1) {
#ifdef DEBUG
    std::cerr << "User " << nickname << " already exists." << std::endl;
#endif
    return -1;
  }
  users.emplace(nickname, (MemberInfo){fd, false, isCreator});
  return 1;
}
int Channel::delUser(std::string nickname) {
  if (getUserInfo(nickname) == -1) return -1;

  users.erase(nickname);
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

  if (info != NULL) info = &(memberIter->second);
  return 1;
}

const std::unordered_map<std::string, MemberInfo>& Channel::getUsers() const {
  return users;
}

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