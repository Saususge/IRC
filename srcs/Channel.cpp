#include "Channel.hpp"

#include <cstdlib>
#include <sstream>

#include "ClientManagement.hpp"
#include "SessionManagement.hpp"

Channel::Channel(const std::string& channelName)
    : channelName(channelName), topic(""), key(""), mode(0), limit(0) {}

Channel::~Channel() {}

const std::string& Channel::getChannelName() const { return channelName; }

int Channel::broadcast(const std::string& msg, const std::string& except) {
  (void)msg;
  (void)except;
  return 1;
}

int Channel::broadcast(const std::string& msg, const ClientID except) {
  for (std::set<ClientID>::iterator iter = joinedUsers.begin();
       iter != joinedUsers.end(); iter++) {
    if (*iter == except) continue;
    SessionManagement::getSession(*iter)->send(msg);
  }
  return 1;
}

IRC::Numeric Channel::addClient(ClientID id, const std::string& key) {
  if (joinedUsers.find(id) != joinedUsers.end())
    return IRC::DO_NOTHING;
  else if (this->mode & IChannel::MINVITE &&
           invitedUsers.find(id) == invitedUsers.end()) {
    return IRC::ERR_INVITEONLYCHAN;
  } else if (!this->key.empty() && this->key != key) {
    return IRC::ERR_BADCHANNELKEY;
  } else if (this->limit != 0 && this->limit <= this->joinedUsers.size()) {
    return IRC::ERR_CHANNELISFULL;
  }

  if (joinedUsers.size() == 0) operators.insert(id);
  joinedUsers.insert(id);
  if (invitedUsers.find(id) != invitedUsers.end()) this->invitedUsers.erase(id);
  return topic.empty() ? IRC::RPL_NOTOPIC : IRC::RPL_TOPIC;
}

IRC::Numeric Channel::removeClient(const ClientID id) {
  if (joinedUsers.find(id) == joinedUsers.end()) return IRC::ERR_NOTONCHANNEL;

  joinedUsers.erase(id);
  return IRC::RPL_STRREPLY;  // <prefix> PART <channel> :<comment>
}
IRC::Numeric Channel::kickClient(ClientID requesterID, ClientID targetID) {
  if (joinedUsers.find(requesterID) == joinedUsers.end())
    return IRC::ERR_NOTONCHANNEL;
  if (operators.find(requesterID) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;
  if (joinedUsers.find(targetID) == joinedUsers.end())
    return IRC::ERR_USERNOTINCHANNEL;

  joinedUsers.erase(targetID);
  // should broadcast quit to other clients including requester.
  // killed client should receive <prefix> KICK <requester> :<comment>
  return IRC::RPL_STRREPLY;
}

bool Channel::hasClient(const ClientID id) const {
  return joinedUsers.find(id) != joinedUsers.end();
}

IRC::Numeric Channel::setClientOp(const ClientID id, const ClientID targetID) {
  std::set<ClientID>::iterator joinIter = joinedUsers.find(id);
  std::set<ClientID>::iterator opIter = operators.find(id);

  // It is not a problem to send ERR_NOTONCHANNEL. But RFC 2812 does not specify
  // numeric reply.
  if (joinIter == joinedUsers.end() || opIter == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  joinIter = joinedUsers.find(targetID);
  if (joinIter == joinedUsers.end()) return IRC::ERR_USERNOTINCHANNEL;

  operators.insert(targetID);
  return IRC::RPL_STRREPLY;  // <prefix> MODE <channel> +o <target>
}

IRC::Numeric Channel::unsetClientOp(const ClientID id,
                                    const ClientID targetid) {
  std::set<ClientID>::iterator joinIter = joinedUsers.find(id);
  std::set<ClientID>::iterator opIter = operators.find(id);

  // It is not a problem to send ERR_NOTONCHANNEL. But RFC 2812 does not specify
  // numeric reply.
  if (joinIter == joinedUsers.end() || opIter == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  joinIter = joinedUsers.find(targetid);
  if (joinIter == joinedUsers.end()) return IRC::ERR_USERNOTINCHANNEL;

  opIter = operators.find(targetid);
  if (opIter == operators.end()) return IRC::DO_NOTHING;

  operators.erase(targetid);
  return IRC::RPL_STRREPLY;  // <prefix> MODE <channel> -o <target>
}

bool Channel::isClientOp(const ClientID id) const {
  if (joinedUsers.find(id) == joinedUsers.end()) return false;

  return operators.find(id) != operators.end();
}

const std::set<ClientID> Channel::getJoinedClients() const {
  return joinedUsers;
}

int Channel::getClientNumber() const { return joinedUsers.size(); }

IRC::Numeric Channel::setMode(ClientID reqeusterid, IChannelMode mode,
                              std::vector<std::string> params) {
  (void)reqeusterid;
  (void)mode;
  (void)params;
  return IRC::DO_NOTHING;
}

IRC::Numeric Channel::addMode(ClientID requesterID, IChannelMode mode,
                              const std::string& params) {
  if (operators.find(requesterID) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  if (mode & IChannel::MINVITE) {
    if (this->mode & IChannel::MINVITE) return IRC::DO_NOTHING;
    this->mode |= IChannel::MINVITE;

  } else if (mode & IChannel::MTOPIC) {
    if (this->mode & IChannel::MTOPIC) return IRC::DO_NOTHING;
    this->mode |= IChannel::MTOPIC;

  } else if (mode & IChannel::MKEY) {
    if (!this->key.empty()) return IRC::ERR_KEYSET;
    if (params.empty()) return IRC::ERR_NEEDMOREPARAMS;
    this->mode |= IChannel::MKEY;
    this->key = params;

  } else if (mode & IChannel::MOP) {
    if (params.empty()) return IRC::ERR_NEEDMOREPARAMS;
    return this->setClientOp(requesterID,
                             ClientManagement::getClientID(params));

  } else if (mode & IChannel::MLIMIT) {
    if (this->limit != 0) return IRC::DO_NOTHING;
    if (params.empty()) return IRC::ERR_NEEDMOREPARAMS;

    this->mode |= IChannel::MLIMIT;

    char* end;
    long num_limit = std::strtol(params.c_str(), &end, 10);
    if (*end != '\0' || num_limit < 0) return IRC::DO_NOTHING;
    if ((size_t)num_limit < joinedUsers.size() ||
        IChannel::MAXUSER < (size_t)num_limit)
      return IRC::DO_NOTHING;

    this->limit = static_cast<size_t>(num_limit);
  } else {
    return IRC::DO_NOTHING;
  }
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::removeMode(ClientID id, IChannelMode mode,
                                 const std::string& params) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;

  if (mode & IChannel::MINVITE) {
    if (!(this->mode & IChannel::MINVITE)) return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MINVITE;

  } else if (mode & IChannel::MTOPIC) {
    if (!(this->mode & IChannel::MTOPIC)) return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MTOPIC;

  } else if (mode & IChannel::MKEY) {
    if (this->key.empty() || params.empty() || this->key != params)
      return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MKEY;
    this->key.clear();

  } else if (mode & IChannel::MOP) {
    if (params.empty()) return IRC::ERR_NEEDMOREPARAMS;
    return this->unsetClientOp(id, ClientManagement::getClientID(params));

  } else if (mode & IChannel::MLIMIT) {
    if (this->limit == 0) return IRC::DO_NOTHING;
    this->mode &= ~IChannel::MLIMIT;
    this->limit = 0;

  } else {
    return IRC::DO_NOTHING;
  }
  return IRC::RPL_STRREPLY;
}

const std::string Channel::getMode() {
  std::string modeStr("+");

  if (mode & IChannel::MINVITE) {
    modeStr += "i";
  }
  if (mode & IChannel::MTOPIC) {
    modeStr += "t";
  }
  if (mode & IChannel::MKEY) {
    modeStr += "k";
  }
  if (mode & IChannel::MLIMIT) {
    std::stringstream ss;

    ss << limit;
    modeStr += "l ";
    modeStr += ss.str();
  }
  return modeStr.size() == 1 ? "" : modeStr;
}

IRC::Numeric Channel::addToInviteList(const ClientID requesterid,
                                      const ClientID targetid) {
  if (joinedUsers.find(requesterid) == joinedUsers.end()) {
    return IRC::ERR_NOTONCHANNEL;
  } else if (joinedUsers.find(targetid) == joinedUsers.end()) {
    return IRC::ERR_USERONCHANNEL;
  } else if (mode & IChannel::MINVITE &&
             operators.find(requesterid) == operators.end()) {
    return IRC::ERR_CHANOPRIVSNEEDED;
  }

  invitedUsers.insert(targetid);
  return IRC::RPL_INVITING;
}

bool Channel::isInInviteList(const ClientID id) const {
  return invitedUsers.find(id) != invitedUsers.end();
}

IRC::Numeric Channel::setTopic(const ClientID id, const std::string& topic) {
  if (joinedUsers.find(id) == joinedUsers.end())
    return IRC::ERR_NOTONCHANNEL;
  else if (mode & IChannel::MTOPIC && operators.find(id) == operators.end())
    return IRC::ERR_CHANOPRIVSNEEDED;

  this->topic = topic;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::reqTopic(const ClientID id) {
  if (joinedUsers.find(id) == joinedUsers.end()) return IRC::ERR_NOTONCHANNEL;
  return topic.empty() ? IRC::RPL_NOTOPIC : IRC::RPL_TOPIC;
}

const std::string& Channel::getTopic() const { return topic; }

bool Channel::isLimited() const { return (this->mode & IChannel::MLIMIT) != 0; }

int Channel::getMaxMember() const { return static_cast<int>(this->limit); }

bool Channel::isInviteOnly() const {
  return (this->mode & IChannel::MINVITE) != 0;
}

bool Channel::isTopicOpOnly() const {
  return (this->mode & IChannel::MTOPIC) != 0;
}

bool Channel::hasKey() const { return (this->mode & IChannel::MKEY) != 0; }

IRC::Numeric Channel::setInviteOnly(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode |= IChannel::MINVITE;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::unsetInviteOnly(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode &= ~IChannel::MINVITE;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::setTopicOpOnly(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode |= IChannel::MTOPIC;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::unsetTopicOpOnly(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode &= ~IChannel::MTOPIC;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::setKey(ClientID id, const std::string& newKey) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  if (newKey.empty()) return IRC::ERR_NEEDMOREPARAMS;
  this->mode |= IChannel::MKEY;
  this->key = newKey;
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::unsetKey(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode &= ~IChannel::MKEY;
  this->key.clear();
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::setLimit(ClientID id, int limit) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  if (limit < 0 || static_cast<size_t>(limit) > IChannel::MAXUSER)
    return IRC::DO_NOTHING;
  this->mode |= IChannel::MLIMIT;
  this->limit = static_cast<size_t>(limit);
  return IRC::RPL_STRREPLY;
}

IRC::Numeric Channel::unsetLimit(ClientID id) {
  if (operators.find(id) == operators.end()) return IRC::ERR_CHANOPRIVSNEEDED;
  this->mode &= ~IChannel::MLIMIT;
  this->limit = 0;
  return IRC::RPL_STRREPLY;
}
