#include "Channel.hpp"

Channel::Channel() {
  inviteOnly = false;
  restrictTopic = false;
  topic = "";
  key = "";
  userLimit = 0;
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

const std::string Channel::addUser(int fd, Client* client, bool isCreator, std::string key) {
  if (getUserInfo(client->getNickname()) == 1) {
#ifdef DEBUG
    std::cerr << "User " << client->getNickname() << " already exists."
              << std::endl;
#endif
    return IRC::DO_NOTHING;  // DO NOTHING
  }

  if (inviteOnly == true) {
    if (invitedUsers.find(client->getNickname()) == invitedUsers.end()) {
#ifdef DEBUG
      std::cerr << "User " << client->getNickname() << " wasn't invited."
                << std::endl;
#endif
      return IRC::ERR_INVITEONLYCHAN;  // ERR_INVITEONLYCHAN
    }
  }

  if (this->key != "" && this->key != key) {
#ifdef DEBUG
    std::cerr << "The channel key " << key << " is wrong." << std::endl;
#endif
    return IRC::ERR_BADCHANNELKEY;  // ERR_BADCHANNELKEY
  }

  if (userLimit != 0 && users.size() >= userLimit) {
#ifdef DEBUG
    std::cerr << "The channel is full. Channel limit: " << userLimit
              << std::endl;
#endif
    return IRC::ERR_CHANNELISFULL;  // ERR_CHANNELISFULL
  }

  if (invitedUsers.find(client->getNickname()) != invitedUsers.end())
    invitedUsers.erase(client->getNickname());

  users.insert(std::pair<std::string, MemberInfo>(
      client->getNickname(), MemberInfo(fd, client, isCreator)));
#ifdef DEBUG
  std::cerr << "User " << client->getNickname() << " joined to the channel."
            << std::endl;
#endif
  if (topic == "")  return IRC::RPL_NOTOPIC;
  return IRC::RPL_TOPIC;  // RPL_TOPIC
}

const std::string Channel::delUser(Client* client) {
  if (getUserInfo(client->getNickname()) == -1) return IRC::ERR_NOTONCHANNEL;  // ERR_NOTONCHANNEL

  users.erase(client->getNickname());
  return "PART";  // send PART #<channel> [:part message]
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

  if (getUserInfo(target, &info) == -1) {
#ifdef DEBUG
    std::cerr << "User " << target << " is not on channel." << std::endl;
#endif
    return -1;  // ERR_NOTONCHANNEL
  }

  users.find(target)->second.op = true;
  return 1;
}

int Channel::setInvite(Client* client, bool value) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname() << " is not in channel."
              << std::endl;
#endif
    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname()
              << " is not a channel operator." << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  if (value == inviteOnly) return 0;  // do nothing

  inviteOnly = value;
  return 1;
}

bool Channel::getInvite() const { return inviteOnly; }

// verify the target is on server before calling
int Channel::inviteUser(Client* client, std::string target) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname() << " is not in channel."
              << std::endl;
#endif
    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname()
              << " is not a channel operator." << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  if (getUserInfo(target) == 1) {
#ifdef DEBUG
    std::cerr << "the target user " << target << " is already on channel."
              << std::endl;
#endif
    return 0;  // do nothing
  }

  invitedUsers.insert(target);
  return 1;
}

int Channel::setTopicMode(Client* client, bool value) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname() << " is not in channel."
              << std::endl;
#endif
    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "the user " << client->getNickname()
              << " is not a channel operator." << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  restrictTopic = value;
  return 1;  // prefix MODE <channel> <param>
}

bool Channel::getTopicMode() const { return restrictTopic; }

int Channel::setTopic(Client* client, std::string topic) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname() << " is not in channel."
              << std::endl;
#endif
    return -1;  // ERR_NOTONCHANNEL
  }

  if (restrictTopic == true && info.op == false) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname()
              << " is not a channel operator." << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  this->topic = topic;
  if (topic == "") return 1;  // RPL_NOTOPIC
  return 1;                   // RPL_TOPIC
}

std::string Channel::getTopic() const { return topic; }

int Channel::setKey(Client* client, std::string newKey) {
  MemberInfo info;
  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname() << " is not on channel"
              << std::endl;
#endif

    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname()
              << " is not a channel operator" << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }
  key = newKey;
  return 1;  // :prefix MODE <channel> <param>
}

std::string Channel::getKey() const { return key; }

int Channel::setUserLimit(Client* client, size_t newLimit) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname() << " is not on channel"
              << std::endl;
#endif

    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname()
              << " is not a channel operator" << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  if (newLimit < users.size() || (newLimit == 0 || 65535 <= newLimit)) {
#ifdef DEBUG
    std::cerr << "New limit is lower than current number of channel users or "
                 "invalid value"
              << newLimit << std::endl;
#endif
    return -1;  // <prefix> 696 <nickname> <channel> l * :Invalid mode parameter
  }

  userLimit = newLimit;
  return 1;  // prefix MODE <channle> +l <newLimit>
}

int Channel::unlimitUser(Client* client) {
  MemberInfo info;

  if (getUserInfo(client->getNickname(), &info) == -1) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname() << " is not on channel"
              << std::endl;
#endif

    return -1;  // ERR_NOTONCHANNEL
  }

  if (info.op == false) {
#ifdef DEBUG
    std::cerr << "The user " << client->getNickname()
              << " is not a channel operator" << std::endl;
#endif
    return -1;  // ERR_CHANOPRIVSNEEDED
  }

  userLimit = 0;
  return 0;
}

int Channel::getUserLimit() const { return userLimit; }
