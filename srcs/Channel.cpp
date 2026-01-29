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
}

bool Channel::hasClient(const std::string& nick) const {}

IRC::Numeric Channel::setClientOp(const std::string& nick,
                                  const std::string& targetNick) {}

IRC::Numeric Channel::unsetClientOp(const std::string& nick,
                                    const std::string& targetNick) {}

bool Channel::isClientOp(const std::string& nick) const {}

const std::vector<const std::string>& Channel::getClients() {}

int Channel::getClientNumber() const {}

IRC::Numeric Channel::setMode(const std::string& reqeusterNick,
                              IChannelMode mode) {}

IRC::Numeric Channel::addMode(const std::string& reqeusterNick,
                              IChannelMode mode) {}

IRC::Numeric Channel::removeMode(const std::string& reqeusterNick,
                                 IChannelMode mode) {}

IRC::Numeric Channel::addToInviteList(const std::string& requesterNick,
                                      const std::string& targetNick) {}

IRC::Numeric Channel::removeFromInviteList(const std::string& requesterNick,
                                           const std::string& targetNick) {}

bool Channel::isInInviteList(const std::string& nick) const {}
