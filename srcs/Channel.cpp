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

int Channel::addClient(const std::string& nick) {
  if (joinedUsers.find(nick) != joinedUsers.end()) return 0;
  joinedUsers.insert(nick);
  return 1;
}

IRC::Numeric Channel::removeClient(const std::string& nick) {}

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
