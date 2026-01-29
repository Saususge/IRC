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

  int addClient(const std::string& nick);
  void removeClient(const std::string& nick);
  bool hasClient(const std::string& nick) const;
  int setClientOp(const std::string& nick);
  int unsetClientOp(const std::string& nick);
  int isClientOp(const std::string& nick) const;
  const std::vector<const std::string>& getClients();

  int getClientNumber() const;

  int setMode(const std::string& reqeusterNick, IChannelMode mode);
  int addMode(const std::string& reqeusterNick, IChannelMode mode);
  int removeMode(const std::string& reqeusterNick, IChannelMode mode);
  IChannelMode getMode() const;

  int addToInviteList(const std::string& requesterNick,
                      const std::string& targetNick);
  int removeFromInviteList(const std::string& requesterNick,
                           const std::string& targetNick);
  bool isInInviteList(const std::string& nick) const;

 private:
  std::string channelName;
  IChannel::IChannelMode mode;
  size_t limit;

  std::set<std::string> invitedUsers;
  std::set<std::string> joinedUsers;
  std::set<std::string> operators;

  IClientRegistry& clientRegistry;
};

#endif
