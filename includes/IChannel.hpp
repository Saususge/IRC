#ifndef ICHANNEL_HPP
#define ICHANNEL_HPP

#include <string>
#include <vector>

#include "numeric.hpp"

class IChannel {
 public:
  // Bit mask
  typedef int IChannelMode;
  static const IChannelMode MINVITE = 0b10000;
  static const IChannelMode MTOPIC = 0b01000;
  static const IChannelMode MKEY = 0b00100;
  static const IChannelMode MOP = 0b00010;
  static const IChannelMode MLIMIT = 0b00001;

  virtual ~IChannel() {};

  virtual const std::string& getChannelName() const = 0;

  // Use `except` only for exclusing the sender
  virtual int broadcast(const std::string& msg,
                        const std::string& except = "") = 0;

  virtual IRC::Numeric addClient(const std::string& nick,
                                 const std::string& key = "") = 0;
  virtual IRC::Numeric removeClient(const std::string& nick) = 0;
  virtual IRC::Numeric kickClient(const std::string& requesterNick,
                                  const std::string& targetNick) = 0;
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual IRC::Numeric setClientOp(const std::string& requesterNick,
                                   const std::string& targetNick) = 0;
  virtual IRC::Numeric unsetClientOp(const std::string& requesterNick,
                                     const std::string& targetNick) = 0;
  virtual bool isClientOp(const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() = 0;

  virtual int getClientNumber() const = 0;

  // MODE +k : blank key not allowed.
  // valid client limit per channel RLIMIT_NOFILE($ulimit -n) / 10
  virtual IRC::Numeric setMode(const std::string& requesterNick,
                               IChannelMode mode) = 0;
  virtual IRC::Numeric addMode(const std::string& requesterNick,
                               IChannelMode mode) = 0;
  virtual IRC::Numeric removeMode(const std::string& requesterNick,
                                  IChannelMode mode) = 0;

  virtual IRC::Numeric addToInviteList(const std::string& requesterNick,
                                       const std::string& targetNick) = 0;
  virtual bool isInInviteList(const std::string& nick) const = 0;

  virtual IRC::Numeric setTopic(const std::string& nick,
                                const std::string& topic) = 0;
  virtual IRC::Numeric reqTopic(const std::string& nick) = 0;
  virtual const std::string& getTopic() = 0;
};
#endif