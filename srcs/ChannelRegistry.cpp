#include "ChannelRegistry.hpp"

IRC::Numeric ChannelRegistry::joinChannel(const std::string& channelName,
                                          const std::string& nick,
                                          const std::string& key) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.addClient(nick, key);
}

IRC::Numeric ChannelRegistry::partChannel(const std::string& channelName,
                                          const std::string& nick) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.removeClient(nick);
}

IRC::Numeric ChannelRegistry::kickChannel(const std::string& channelName,
                                          const std::string& requesterNick,
                                          const std::string& targetNick) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.kickClient(requesterNick, targetNick);
}

bool ChannelRegistry::hasChannel(const std::string& channelName) const {
  return channels.find(channelName) != channels.end();
}

const std::map<std::string, IChannel&>& ChannelRegistry::getChannels() const {
  return channels;
}

IRC::Numeric ChannelRegistry::setClientOp(const std::string& channelName,
                                          const std::string& requesterNick,
                                          const std::string& targetNick) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.setClientOp(requesterNick, targetNick);
}

IRC::Numeric ChannelRegistry::unsetClientOp(const std::string& channelName,
                                            const std::string& requesterNick,
                                            const std::string& targetNick) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.unsetClientOp(requesterNick, targetNick);
}

IRC::Numeric ChannelRegistry::setMode(const std::string& channelName,
                                      const std::string& requesterNick,
                                      IChannel::IChannelMode mode,
                                      std::vector<std::string> params) {
  (void)requesterNick;
  (void)mode;
  (void)params;
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;
  return IRC::DO_NOTHING;
}

IRC::Numeric ChannelRegistry::addMode(const std::string& channelName,
                                      const std::string& requesterNick,
                                      IChannel::IChannelMode mode,
                                      const std::string& param) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;
  return iter->second.addMode(requesterNick, mode, param);
}

IRC::Numeric ChannelRegistry::removeMode(const std::string& channelName,
                                         const std::string& requesterNick,
                                         IChannel::IChannelMode mode,
                                         const std::string& param) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;
  return iter->second.removeMode(requesterNick, mode, param);
}

bool ChannelRegistry::hasClient(const std::string& channelName,
                                const std::string& nick) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return false;

  return iter->second.hasClient(nick);
}

bool ChannelRegistry::isClientOp(const std::string& channelName,
                                 const std::string& nick) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return false;

  return iter->second.isClientOp(nick);
}

// TODO: change interface function. add const std::string& channelName param.
const std::set<std::string>& ChannelRegistry::getClients(
    const std::string& channelName) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      channels.find(channelName);
  if (iter == channels.end()) {
    static const std::set<std::string> emptySet;
    return emptySet;
  }
  return iter->second.getClients();
}

int ChannelRegistry::getClientNumber(const std::string& channelName) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return -1;  // No such channel

  return iter->second.getClientNumber();
}

IRC::Numeric ChannelRegistry::addToInviteList(const std::string& channelName,
                                              const std::string& requesterNick,
                                              const std::string& targetNick) {
  std::map<std::string, IChannel&>::iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return IRC::ERR_NOSUCHCHANNEL;

  return iter->second.addToInviteList(requesterNick, targetNick);
}

bool ChannelRegistry::isInInviteList(const std::string& channelName,
                                     const std::string& nick) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return false;

  return iter->second.isInInviteList(nick);
}

int ChannelRegistry::broadcast(const std::string& channelName,
                               const std::string& msg,
                               const std::string& except) {
  std::map<std::string, IChannel&>::iterator iter =
      this->channels.find(channelName);

  if (iter == channels.end()) return -1;  // No such channel

  return iter->second.broadcast(msg, except);
}

IRC::Numeric ChannelRegistry::setTopic(const std::string& channelName,
                                       const std::string& nick,
                                       const std::string& topic) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) {
    // rfc 2812 topic does not return ERR_NOSUCHCHANNEL
    return IRC::DO_NOTHING;
  }

  return iter->second.setTopic(nick, topic);
}

IRC::Numeric ChannelRegistry::reqTopic(const std::string& channelName,
                                       const std::string& nick) {
  std::map<std::string, IChannel&>::iterator iter = channels.find(channelName);
  if (iter == channels.end()) {
    // rfc 2812 topic does not return ERR_NOSUCHCHANNEL
    return IRC::DO_NOTHING;
  }

  return iter->second.reqTopic(nick);
}

const std::string& ChannelRegistry::getTopic(
    const std::string& channelName) const {
  std::map<std::string, IChannel&>::const_iterator iter =
      channels.find(channelName);
  if (iter == channels.end()) {
    static const std::string error(":ERROR:");
    return error;
  }
  return iter->second.getTopic();
}
