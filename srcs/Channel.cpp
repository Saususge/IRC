#include "Channel.hpp"

#include <cstdlib>

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

  if (joinedUsers.size() == 0) operators.insert(nick);
  joinedUsers.insert(nick);
  if ((mode & 0b10) && (invitedUsers.find(nick) == invitedUsers.end()))
    this->invitedUsers.erase(nick);
  return topic.empty() ? IRC::RPL_NOTOPIC : IRC::RPL_TOPIC;
}

IRC::Numeric Channel::removeClient(const std::string& nick) {
  if (joinedUsers.find(nick) == joinedUsers.end()) return IRC::ERR_NOTONCHANNEL;

  joinedUsers.erase(nick);
  // TODO: call clientRegistry.partChannel();

  return IRC::RPL_STRREPLY;  // <prefix> PART <channel> :<comment>
}
IRC::Numeric Channel::kickClient(const std::string& requesterNick,
                                 const std::string& targetNick) {
  if (joinedUsers.find(requesterNick) == joinedUsers.end())
    return IRC::ERR_NOTONCHANNEL;
  if (operators.find(requesterNick) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;
  if (joinedUsers.find(targetNick) == joinedUsers.end())
    return IRC::ERR_USERNOTINCHANNEL;

  joinedUsers.erase(targetNick);
  // TODO: call clientRegistry.partChannel();
  // should broadcast quit to other clients including requester.
  // killed client should receive <prefix> KILL <requester> :<comment>
  return IRC::RPL_STRREPLY;
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
                              IChannelMode mode,
                              std::vector<const std::string> params) {
  (void)reqeusterNick;
  (void)mode;
  (void)params;
  return IRC::DO_NOTHING;
}

IRC::Numeric Channel::addMode(const std::string& requesterNick,
                              IChannelMode mode, const std::string& param) {
  if (operators.find(requesterNick) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  if (mode & IChannel::MINVITE) {
    if (this->mode & IChannel::MINVITE) return IRC::DO_NOTHING;
    this->mode |= IChannel::MINVITE;

  } else if (mode & IChannel::MTOPIC) {
    if (this->mode & IChannel::MTOPIC) return IRC::DO_NOTHING;
    this->mode |= IChannel::MTOPIC;

  } else if (mode & IChannel::MKEY) {
    if (!this->key.empty()) return IRC::ERR_KEYSET;
    if (param.empty()) return IRC::ERR_NEEDMOREPARAMS;
    this->key = param;

  } else if (mode & IChannel::MOP) {
    if (param.empty()) return IRC::ERR_NEEDMOREPARAMS;
    return this->setClientOp(requesterNick, param);

  } else if (mode & IChannel::MLIMIT) {
    if (this->limit != 0) return IRC::DO_NOTHING;
    if (param.empty()) return IRC::ERR_NEEDMOREPARAMS;

    char* end;
    size_t num_limit =
        static_cast<size_t>(std::strtol(param.c_str(), &end, 10));
    if (*end != '\0' || (num_limit < joinedUsers.size() || 102 < num_limit))
      return IRC::DO_NOTHING;

    this->limit = num_limit;
  } else {
    return IRC::DO_NOTHING;
  }
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::removeMode(const std::string& requesterNick,
                                 IChannelMode mode, const std::string& param) {
  if (operators.find(requesterNick) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  if (mode & IChannel::MINVITE) {
    if (!(this->mode & IChannel::MINVITE)) return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MINVITE;

  } else if (mode & IChannel::MTOPIC) {
    if (!(this->mode & IChannel::MTOPIC)) return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MTOPIC;

  } else if (mode & IChannel::MKEY) {
    if (this->key.empty()) return IRC::DO_NOTHING;
    if (param.empty() || this->key != param) return IRC::ERR_KEYSET;
    this->key.clear();

  } else if (mode & IChannel::MOP) {
    if (param.empty()) return IRC::ERR_NEEDMOREPARAMS;
    return this->unsetClientOp(requesterNick, param);

  } else if (mode & IChannel::MLIMIT) {
    if (this->limit == 0) return IRC::DO_NOTHING;
    this->limit = 0;

  } else {
    return IRC::DO_NOTHING;
  }
  return IRC::RPL_STRREPLY;
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

IRC::Numeric Channel::setTopic(const std::string& nick,
                               const std::string& topic) {
  if (joinedUsers.find(nick) == joinedUsers.end())
    return IRC::ERR_NOTONCHANNEL;
  else if (mode & 0b1 && operators.find(nick) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  this->topic = topic;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::reqTopic(const std::string& nick) {
  if (joinedUsers.find(nick) == joinedUsers.end()) return IRC::ERR_NOTONCHANNEL;
  return topic.empty() ? IRC::RPL_NOTOPIC : IRC::RPL_TOPIC;
}

const std::string& Channel::getTopic() { return topic; }
