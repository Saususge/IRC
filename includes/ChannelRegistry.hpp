#ifndef CHANNELREGISTRY_HPP
#define CHANNELREGISTRY_HPP

#include <iostream>
#include <map>

#include "IChannel.hpp"
#include "IChannelRegistry.hpp"

class ChannelRegistry : public IChannelRegistry {
 public:
  IRC::Numeric joinChannel(const std::string& channelName,
                           const std::string& nick,
                           const std::string& key = "");
  IRC::Numeric partChannel(const std::string& channelName,
                           const std::string& nick);
  IRC::Numeric kickChannel(const std::string& channelName,
                           const std::string& requesterNick,
                           const std::string& targetNick);
  bool hasChannel(const std::string& channelName) const;
  const std::map<std::string, IChannel*>& getChannels() const;
  IRC::Numeric setClientOp(const std::string& channelName,
                           const std::string& requesterNick,
                           const std::string& targetNick);
  IRC::Numeric unsetClientOp(const std::string& channelName,
                             const std::string& requesterNick,
                             const std::string& targetNick);
  IRC::Numeric setMode(const std::string& channelName,
                       const std::string& requesterNick,
                       IChannel::IChannelMode mode,
                       std::vector<std::string> params);
  IRC::Numeric addMode(const std::string& channelName,
                       const std::string& requesterNick,
                       IChannel::IChannelMode mode, const std::string& param);
  IRC::Numeric removeMode(const std::string& channelName,
                          const std::string& requesterNick,
                          IChannel::IChannelMode mode,
                          const std::string& param);

  bool hasClient(const std::string& channelName, const std::string& nick) const;
  bool isClientOp(const std::string& channelName,
                  const std::string& nick) const;
  const std::set<std::string>& getClients(const std::string& channelName) const;

  int getClientNumber(const std::string& channelName) const;

  IRC::Numeric addToInviteList(const std::string& channelName,
                               const std::string& requesterNick,
                               const std::string& targetNick);
  bool isInInviteList(const std::string& channelName,
                      const std::string& nick) const;

  IRC::Numeric setTopic(const std::string& channelName, const std::string& nick,
                        const std::string& topic);
  IRC::Numeric reqTopic(const std::string& channelName,
                        const std::string& nick);
  const std::string& getTopic(const std::string& channelName) const;

  // Use `except` only if excluding sender.
  int broadcast(const std::string& channelName, const std::string& msg,
                const std::string& except = "");

 private:
  std::map<std::string, IChannel*> channels;
};

#endif  // CHANNELREGISTRY_HPP