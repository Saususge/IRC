#include "Channel.hpp"

Channel::Channel() {
  inviteOnly = false;
  restrictTopic = false;
  topic = "";
  key = "";
}

Channel::~Channel() {}

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

int Channel::addUser(int fd, User* user, bool isCreator, std::string key) {
  if (getUserInfo(user->nickname) == 1) {
#ifdef DEBUG
    std::cerr << "User " << user->nickname << " already exists." << std::endl;
#endif
    return 0; // DO NOTHING
  }

  if (this->key != "" && this->key != key)
    return -1; // ERR_BADCHANNELKEY

  if (inviteOnly == true) {
    if (invitedUsers.find(user->nickname) == invitedUsers.end()) {
#ifdef DEBUG
      std::cerr << "User " << user->nickname << " wasn't invited." << std::endl;
#endif
      return -1; // ERR_INVITEONLYCHAN
    }
  }

  users.insert(std::pair<std::string, MemberInfo>(user->nickname, MemberInfo(fd, user, isCreator)));
#ifdef DEBUG
    std::cerr << "User " << user->nickname << " joined to the channel." << std::endl;
#endif
  return 1; // RPL_TOPIC
}

int Channel::delUser(User user) {
  if (getUserInfo(user.nickname) == -1) return -1; // ERR_NOTONCHANNEL

  users.erase(user.nickname);
  return 1; // send PART #<channel> [:part message]
}

int Channel::promoteToOp(User user, std::string target) {
  MemberInfo info;
  if (getUserInfo(user.nickname, &info) == -1) return -1;

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "User " << user.nickname << " is not channel operator."
              << std::endl;
#endif
    return -2;
  }

  if (getUserInfo(target, &info) == -1) return -1;

  info.op = true;
  return 1;
}

int Channel::setInvite(User user, bool value) {
  (void)user;                         // TODO: check fd is operator
  if (value == inviteOnly) return 0;  // do nothing

  inviteOnly = value;
  return 1;
}
bool Channel::getInvite() const { return inviteOnly; }

int Channel::inviteUser(User user, std::string target) {
  MemberInfo info;

  (void)user;  // TODO: check fd is operator

  if (getUserInfo(target, &info) == -1) return -1;

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "User " << user.nickname << " is not operator." << std::endl;
#endif
    return -2;
  }
  invitedUsers.insert(target);
  return 1;
}

void Channel::setTopicMode(User user, bool value) {  // not implemented
  (void)user;
  (void)value;
}

bool Channel::getTopicMode() const { return restrictTopic; }

void Channel::setTopic(User user, std::string topic) {  // not implemented
  (void)user;
  (void)topic;
}

std::string Channel::getTopic() const { return topic; }

int Channel::setKey(User user, std::string newKey) {  // not implemented
  MemberInfo info;
  if (getUserInfo(user.nickname, &info) == -1) {
    #ifdef DEBUG
      std::cerr << "The user " << user.nickname << " is not on channel" << std::endl;
    #endif

    return -1; // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
    #ifdef DEBUG
      std::cerr << "The user " << user.nickname << " is not a channel operator" << std::endl;
    #endif
    return -1; // ERR_CHANOPRIVSNEEDED
  }
  key = newKey;
  return 1; // :prefix MODE <channel> <param>
}

std::string Channel::getKey() const { return key; }
