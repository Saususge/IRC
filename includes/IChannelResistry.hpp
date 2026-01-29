#ifndef ICHANNELRESISTRY_HPP
#define ICHANNELRESISTRY_HPP

#include <string>
#include <vector>

#include "IChannel.hpp"

class IChannelResistry {
 public:
  virtual ~IChannelResistry() {};

  virtual int createChannel(const std::string& channelName) = 0;
  virtual int deleteChannel(const std::string& channelName) = 0;
  virtual bool hasChannel(const std::string& channelName) = 0;
  virtual const std::vector<const std::string>& getChannels() = 0;
  virtual int setClientOp(const std::string& channelName,
                          const std::string& nick) = 0;
  virtual int unsetClientOp(const std::string& channelName,
                            const std::string& nick) = 0;
  virtual int setMode(const std::string& channelName,
                      const std::string requesterNick,
                      IChannel::IChannelMode mode) = 0;
  virtual int addMode(const std::string& channelName,
                      const std::string requesterNick,
                      IChannel::IChannelMode mode) = 0;
  virtual int removeMode(const std::string& channelName,
                         const std::string requesterNick,
                         IChannel::IChannelMode mode) = 0;

  virtual int removeClient(const std::string& channelName,
                           const std::string& nick) = 0;
  virtual bool hasClient(const std::string& channelName,
                         const std::string& nick) const = 0;
  virtual int isClientOp(const std::string& channelName,
                         const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() = 0;

  virtual int getClientNumber(const std::string& channelName) const = 0;

  virtual int setMode(const std::string& channelName,
                      const std::string& reqeusterNcik,
                      IChannel::IChannelMode mode) = 0;
  virtual int addMode(const std::string& channelName,
                      const std::string& reqeusterNcik,
                      IChannel::IChannelMode mode) = 0;
  virtual int removeMode(const std::string& channelName,
                         const std::string& reqeusterNcik,
                         IChannel::IChannelMode mode) = 0;

  virtual int addToInviteList(const std::string& channelName,
                              const std::string& requesterNcik,
                              const std::string& targetNick) = 0;
  virtual int removeFromInviteList(const std::string& channelName,
                                   const std::string& requesterNcik,
                                   const std::string& targetNick) = 0;
  virtual bool isInInviteList(const std::string& channelName,
                              const std::string& nick) const = 0;

  // Use `except` only if excluding sender.
  virtual int broadcast(const std::string& channelName, const std::string& msg,
                        const std::string& except = "") = 0;
};
#endif  // ICHANNELRESISTRY_HPP