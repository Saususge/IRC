#ifndef ICHANNELRESISTRY_HPP
#define ICHANNELRESISTRY_HPP

#include <string>
#include <vector>

#include "IChannel.hpp"
#include "numeric.hpp"

class IChannelRegistry {
 public:
  virtual ~IChannelRegistry() {};

  virtual IRC::Numeric joinChannel(const std::string& channelName,
                                   const std::string& nick,
                                   const std::string& key = "") = 0;
  virtual IRC::Numeric partChannel(const std::string& channelName,
                                   const std::string& nick) = 0;
  virtual bool hasChannel(const std::string& channelName) = 0;
  virtual const std::vector<const std::string>& getChannels() = 0;
  virtual IRC::Numeric setClientOp(const std::string& channelName,
                                   const std::string& requesterNick,
                                   const std::string& targetNick) = 0;
  virtual IRC::Numeric unsetClientOp(const std::string& channelName,
                                     const std::string& requesterNick,
                                     const std::string& targetNick) = 0;
  virtual IRC::Numeric setMode(const std::string& channelName,
                               const std::string requesterNick,
                               IChannel::IChannelMode mode) = 0;
  virtual IRC::Numeric addMode(const std::string& channelName,
                               const std::string requesterNick,
                               IChannel::IChannelMode mode) = 0;
  virtual IRC::Numeric removeMode(const std::string& channelName,
                                  const std::string requesterNick,
                                  IChannel::IChannelMode mode) = 0;

  virtual IRC::Numeric removeClient(const std::string& channelName,
                                    const std::string& nick) = 0;
  virtual bool hasClient(const std::string& channelName,
                         const std::string& nick) const = 0;
  virtual bool isClientOp(const std::string& channelName,
                          const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() = 0;

  virtual int getClientNumber(const std::string& channelName) const = 0;

  virtual IRC::Numeric setMode(const std::string& channelName,
                               const std::string& requesterNick,
                               IChannel::IChannelMode mode) = 0;
  virtual IRC::Numeric addMode(const std::string& channelName,
                               const std::string& requesterNick,
                               IChannel::IChannelMode mode) = 0;
  virtual IRC::Numeric removeMode(const std::string& channelName,
                                  const std::string& requesterNick,
                                  IChannel::IChannelMode mode) = 0;

  virtual IRC::Numeric addToInviteList(const std::string& channelName,
                                       const std::string& requesterNick,
                                       const std::string& targetNick) = 0;
  virtual bool isInInviteList(const std::string& channelName,
                              const std::string& nick) const = 0;

  // Use `except` only if excluding sender.
  virtual int broadcast(const std::string& channelName, const std::string& msg,
                        const std::string& except = "") = 0;
};
#endif  // ICHANNELRESISTRY_HPP