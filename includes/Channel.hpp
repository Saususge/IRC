#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>

#include "IChannel.hpp"
#include "IClientRegistry.hpp"

class Channel : private IChannel {
 public:
  const std::string& getChannelName() const;

  // Use `except` only for exclusing the sender
  int broadcast(const std::string& msg, const std::string& except = "");

  IRC::Numeric addClient(const std::string& nick, const std::string& key = "");
  IRC::Numeric removeClient(const std::string& nick);
  bool hasClient(const std::string& nick) const;
  IRC::Numeric setClientOp(const std::string& requesterNick,
                           const std::string& targetNick);
  IRC::Numeric unsetClientOp(const std::string& requesterNick,
                             const std::string& targetNick);
  bool isClientOp(const std::string& nick) const;
  const std::vector<const std::string>& getClients();

  int getClientNumber() const;

  IRC::Numeric setMode(const std::string& requesterNick, IChannelMode mode);
  IRC::Numeric addMode(const std::string& requesterNick, IChannelMode mode);
  IRC::Numeric removeMode(const std::string& requesterNick, IChannelMode mode);

  IRC::Numeric addToInviteList(const std::string& requesterNick,
                               const std::string& targetNick);
  IRC::Numeric removeFromInviteList(const std::string& requesterNick,
                                    const std::string& targetNick);
  bool isInInviteList(const std::string& nick) const;

 private:
  std::string channelName;
  std::string topic;
  std::string key;
  IChannel::IChannelMode mode;  // inv, topic
  size_t limit;

  std::set<std::string> invitedUsers;
  std::set<std::string> joinedUsers;
  std::set<std::string> operators;

  IClientRegistry& clientRegistry;
};

#endif
