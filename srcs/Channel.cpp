#include "Channel.hpp"

int Channel::broadcast(const std::string& msg, const std::string& except = "") {
  if (!except.empty()) joinedUsers.erase(except);
  for (std::set<std::string>::iterator iter = joinedUsers.begin();
       iter != joinedUsers.end(); iter++) {
    this->clientRegistry.send(*iter, msg);
  }
  if (!except.empty()) joinedUsers.insert(except);
  return 1;
}

IRC::Numeric Channel::addClient(const std::string& nick,
                                const std::string& key) {
  if (joinedUsers.find(nick) != joinedUsers.end())
    return IRC::DO_NOTHING;
  else if (this->mode & 0b10 && invitedUsers.find(nick) == invitedUsers.end()) {
    return IRC::ERR_INVITEONLYCHAN;
  } else if (!this->key.empty() && this->key != key) {
    return IRC::ERR_BADCHANNELKEY;
  } else if (this->limit != 0 && this->limit <= this->joinedUsers.size()) {
    return IRC::ERR_CHANNELISFULL;
  }

  joinedUsers.insert(nick);
  this->mode & 0b10 ? this->invitedUsers.erase(nick) : 0;

  return topic.empty() ? IRC::RPL_NOTOPIC : IRC::RPL_TOPIC;
}

IRC::Numeric Channel::removeClient(const std::string& nick) {
  if (joinedUsers.find(nick) == joinedUsers.end()) return IRC::ERR_NOTONCHANNEL;
  joinedUsers.erase(nick);
  return IRC::RPL_STRREPLY;  // <prefix> PART <channel> :<comment>
}

bool Channel::hasClient(const std::string& nick) const {
  return joinedUsers.find(nick) != joinedUsers.end();
}

IRC::Numeric Channel::setClientOp(const std::string& nick,
                                  const std::string& targetNick) {
  std::set<std::string>::iterator joinIter = joinedUsers.find(nick);
  std::set<std::string>::iterator opIter = operators.find(nick);

  // It is not a problem to send ERR_NOTONCHANNEL. But RFC 2812 does not specify
  // numeric reply.
  if (joinIter == joinedUsers.end() || opIter == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  joinIter = joinedUsers.find(targetNick);
  if (joinIter == joinedUsers.end()) return IRC::ERR_USERNOTINCHANNEL;

  operators.insert(targetNick);
  return IRC::RPL_STRREPLY;  // <prefix> MODE <channel> +o <target>
}

IRC::Numeric Channel::unsetClientOp(const std::string& nick,
                                    const std::string& targetNick) {
  std::set<std::string>::iterator joinIter = joinedUsers.find(nick);
  std::set<std::string>::iterator opIter = operators.find(nick);

  // It is not a problem to send ERR_NOTONCHANNEL. But RFC 2812 does not specify
  // numeric reply.
  if (joinIter == joinedUsers.end() || opIter == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  joinIter = joinedUsers.find(targetNick);
  if (joinIter == joinedUsers.end()) return IRC::ERR_USERNOTINCHANNEL;

  opIter = operators.find(targetNick);
  if (opIter == operators.end()) return IRC::DO_NOTHING;

  operators.erase(targetNick);
  return IRC::RPL_STRREPLY;  // <prefix> MODE <channel> -o <target>
}

bool Channel::isClientOp(const std::string& nick) const {
  if (joinedUsers.find(nick) == joinedUsers.end()) return false;

  return operators.find(nick) != operators.end();
}

const std::vector<const std::string>& Channel::getClients() {
  const std::vector<const std::string> v(joinedUsers.begin(),
                                         joinedUsers.end());
  return v;
}

int Channel::getClientNumber() const { return joinedUsers.size(); }

IRC::Numeric Channel::setMode(const std::string& reqeusterNick,
                              IChannelMode mode) {
  (void)reqeusterNick;
  (void)mode;
  return IRC::DO_NOTHING;
}

IRC::Numeric Channel::addMode(const std::string& reqeusterNick,
                              IChannelMode mode) {
  (void)reqeusterNick;
  (void)mode;
  return IRC::DO_NOTHING;
}

IRC::Numeric Channel::removeMode(const std::string& reqeusterNick,
                                 IChannelMode mode) {
  (void)reqeusterNick;
  (void)mode;
  return IRC::DO_NOTHING;
}

IRC::Numeric Channel::addToInviteList(const std::string& requesterNick,
                                      const std::string& targetNick) {
  if (joinedUsers.find(requesterNick) == joinedUsers.end()) {
    return IRC::ERR_NOTONCHANNEL;
  } else if (joinedUsers.find(targetNick) == joinedUsers.end()) {
    return IRC::ERR_USERONCHANNEL;
  } else if (mode & 0b10 && operators.find(requesterNick) == operators.end()) {
    return IRC::ERR_CHANOPRIVSNEEDED;
  }

  invitedUsers.insert(targetNick);
  return IRC::RPL_INVITING;
}

bool Channel::isInInviteList(const std::string& nick) const {
  return invitedUsers.find(nick) != invitedUsers.end();
}
