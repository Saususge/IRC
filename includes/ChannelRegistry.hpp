#ifndef CHANNELREGISTRY_HPP
#define CHANNELREGISTRY_HPP

#include <iostream>
#include <map>
#include <vector>

#include "IChannel.hpp"
#include "IChannelRegistry.hpp"

class ChannelRegistry : private IChannelRegistry {
 public:
  IRC::Numeric joinChannel(const std::string& channelName,
                           const std::string& nick);
  IRC::Numeric partChannel(const std::string& channelName,
                           const std::string& nick);
  IRC::Numeric kickChannel(const std::string& channelName,
                           const std::string& requesterNick,
                           const std::string& targetNick);
  bool hasChannel(const std::string& channelName);
  const std::vector<const std::string>& getChannels();
  IRC::Numeric setClientOp(const std::string& channelName,
                           const std::string& requesterNick,
                           const std::string& targetNick);
  IRC::Numeric unsetClientOp(const std::string& channelName,
                             const std::string& requesterNick,
                             const std::string& targetNick);
  IRC::Numeric setMode(const std::string& channelName,
                       const std::string requesterNick,
                       IChannel::IChannelMode mode,
                       std::vector<const std::string> params);
  IRC::Numeric addMode(const std::string& channelName,
                       const std::string requesterNick,
                       IChannel::IChannelMode mode, const std::string& param);
  IRC::Numeric removeMode(const std::string& channelName,
                          const std::string requesterNick,
                          IChannel::IChannelMode mode,
                          const std::string& param);

  bool hasClient(const std::string& channelName, const std::string& nick) const;
  bool isClientOp(const std::string& channelName,
                  const std::string& nick) const;
  const std::vector<const std::string>& getClients();

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
  const std::string& getTopic(const std::string& channelName);

  // Use `except` only if excluding sender.
  int broadcast(const std::string& channelName, const std::string& msg,
                const std::string& except = "");

 private:
  std::map<std::string, IChannel> channels;
};

#endif  // CHANNELREGISTRY_HPP