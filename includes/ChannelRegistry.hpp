#ifndef CHANNELREGISTRY_HPP
#define CHANNELREGISTRY_HPP

#include "IChannelRegistry.hpp"

class ChannelRegistry : private IChannelRegistry {
 public:
  int createChannel(const std::string& channelName);
  int deleteChannel(const std::string& channelName);
  bool hasChannel(const std::string& channelName);
  const std::vector<const std::string>& getChannels();
  int setClientOp(const std::string& channelName, const std::string& nick);
  int unsetClientOp(const std::string& channelName, const std::string& nick);
  int setMode(const std::string& channelName, const std::string requesterNick,
              IChannel::IChannelMode mode);
  int addMode(const std::string& channelName, const std::string requesterNick,
              IChannel::IChannelMode mode);
  int removeMode(const std::string& channelName,
                 const std::string requesterNick, IChannel::IChannelMode mode);

  int removeClient(const std::string& channelName, const std::string& nick);
  bool hasClient(const std::string& channelName, const std::string& nick) const;
  int isClientOp(const std::string& channelName, const std::string& nick) const;
  const std::vector<const std::string>& getClients();

  int getClientNumber(const std::string& channelName) const;

  int setMode(const std::string& channelName, const std::string& reqeusterNick,
              IChannel::IChannelMode mode);
  int addMode(const std::string& channelName, const std::string& reqeusterNick,
              IChannel::IChannelMode mode);
  int removeMode(const std::string& channelName,
                 const std::string& reqeusterNick, IChannel::IChannelMode mode);
  // Use negative value to return error
  IChannel::IChannelMode getMode(const std::string& channelName) const;

  int addToInviteList(const std::string& channelName,
                      const std::string& requesterNick,
                      const std::string& targetNick);
  int removeFromInviteList(const std::string& channelName,
                           const std::string& requesterNick,
                           const std::string& targetNick);
  bool isInInviteList(const std::string& channelName,
                      const std::string& nick) const;

  // Use `except` only if excluding sender.
  int broadcast(const std::string& channelName, const std::string& msg,
                const std::string& except = "");

 private:
};

#endif  // CHANNELREGISTRY_HPP