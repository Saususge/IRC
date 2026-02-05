#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <vector>

#include "IChannel.hpp"
#include "IClientRegistry.hpp"

class Channel : public IChannel {
 public:
  Channel();
  Channel(const std::string& channelName, IClientRegistry& clientRegistry);
  ~Channel();
  const std::string& getChannelName() const;

  // Use `except` only for exclusing the sender
  int broadcast(const std::string& msg, const std::string& except = "");

  IRC::Numeric addClient(const std::string& nick, const std::string& key = "");
  IRC::Numeric removeClient(const std::string& nick);
  IRC::Numeric kickClient(const std::string& requesterNick,
                          const std::string& targetNick);
  bool hasClient(const std::string& nick) const;
  IRC::Numeric setClientOp(const std::string& requesterNick,
                           const std::string& targetNick);
  IRC::Numeric unsetClientOp(const std::string& requesterNick,
                             const std::string& targetNick);
  bool isClientOp(const std::string& nick) const;
  const std::set<std::string>& getClients() const;

  int getClientNumber() const;

  IRC::Numeric setMode(const std::string& requesterNick, IChannelMode mode,
                       std::vector<std::string> params);
  IRC::Numeric addMode(const std::string& requesterNick, IChannelMode mode,
                       const std::string& param);
  IRC::Numeric removeMode(const std::string& requesterNick, IChannelMode mode,
                          const std::string& param);
  const std::string getMode();

  IRC::Numeric addToInviteList(const std::string& requesterNick,
                               const std::string& targetNick);
  bool isInInviteList(const std::string& nick) const;

  IRC::Numeric setTopic(const std::string& nick, const std::string& topic);
  IRC::Numeric reqTopic(const std::string& nick);
  const std::string& getTopic() const;

 private:
  std::string channelName;
  std::string topic;
  std::string key;
  IChannel::IChannelMode mode;  // inv, topic, key, limit
  size_t limit;

  std::set<std::string> invitedUsers;
  std::set<std::string> joinedUsers;
  std::set<std::string> operators;

  IClientRegistry& clientRegistry;
};

#endif
