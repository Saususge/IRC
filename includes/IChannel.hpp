#ifndef ICHANNEL_HPP
#define ICHANNEL_HPP

#include <set>
#include <string>
#include <vector>

#include "defs.hpp"
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
  static const size_t MAXUSER = 102;

  virtual ~IChannel() {};

  virtual const std::string& getChannelName() const = 0;

  // Use `except` only for exclusing the sender
  virtual int broadcast(const std::string& msg,
                        const std::string& except = "") = 0;
  virtual int broadcast(const std::string& msg,
                        const ClientID except = ClientID(-1)) = 0;

  virtual IRC::Numeric join(ClientID clientID, const std::string& key = "") = 0;
  virtual IRC::Numeric part(ClientID clientID) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric addClient(const std::string& nick,
                                 const std::string& key = "") = 0;
  virtual IRC::Numeric addClient(ClientID id, const std::string& key = "") = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric removeClient(const std::string& nick) = 0;
  virtual IRC::Numeric removeClient(ClientID id) = 0;
  virtual IRC::Numeric kickClient(ClientID requesterID, ClientID targetID) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric kickClient(const std::string& requesterNick,
                                  const std::string& targetNick) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual bool hasClient(ClientID id) const = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric setClientOp(const std::string& requesterNick,
                                   const std::string& targetNick) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric unsetClientOp(const std::string& requesterNick,
                                     const std::string& targetNick) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual bool isClientOp(const std::string& nick) const = 0;
  virtual bool isClientOp(ClientID id) const = 0;
  virtual const std::set<ClientID> getJoinedClients() const = 0;

  virtual int getClientNumber() const = 0;

  // MODE +k : blank key not allowed.
  // MODE -k : also requires password
  // valid client limit per channel RLIMIT_NOFILE($ulimit -n) / 10 -> 102.4
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric setMode(const std::string& requesterNick,
                               IChannelMode mode,
                               std::vector<std::string> params) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric addMode(const std::string& requesterNick,
                               IChannelMode mode, const std::string& param) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric removeMode(const std::string& requesterNick,
                                  IChannelMode mode,
                                  const std::string& params) = 0;
  virtual IRC::Numeric setMode(ClientID id, IChannelMode mode,
                               std::vector<std::string> params) = 0;
  virtual IRC::Numeric addMode(ClientID id, IChannelMode mode,
                               const std::string& param) = 0;
  virtual IRC::Numeric removeMode(ClientID id, IChannelMode mode,
                                  const std::string& params) = 0;
  virtual const std::string getMode() = 0;

  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric addToInviteList(const std::string& requesterNick,
                                       const std::string& targetNick) = 0;
  virtual IRC::Numeric addToInviteList(ClientID requesterId,
                                       ClientID targetId) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual bool isInInviteList(const std::string& nick) const = 0;
  virtual bool isInInviteList(ClientID id) const = 0;

  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric setTopic(const std::string& nick,
                                const std::string& topic) = 0;
  [[deprecated("Use ClientID overload")]]
  virtual IRC::Numeric reqTopic(const std::string& nick) = 0;
  virtual IRC::Numeric setTopic(ClientID id, const std::string& topic) = 0;
  virtual IRC::Numeric reqTopic(ClientID id) = 0;
  virtual const std::string& getTopic() const = 0;
};
#endif