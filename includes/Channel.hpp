#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <vector>

#include "IChannel.hpp"

class Channel : public IChannel {
 public:
  Channel();
  Channel(const std::string& channelName);
  ~Channel();
  const std::string& getChannelName() const;

  // Use `except` only for exclusing the sender
  int broadcast(const std::string& msg, const std::string& except = "");
  int broadcast(const std::string& msg, const ClientID except = ClientID(-1));

  // IRC::Numeric addClient(const std::string& nick, const std::string& key =
  // ""); IRC::Numeric removeClient(const std::string& nick); IRC::Numeric
  // kickClient(const std::string& requesterNick,
  //                         const std::string& targetNick);

  IRC::Numeric join(ClientID clientID, const std::string& key = "");
  IRC::Numeric part(ClientID clientID);

  IRC::Numeric addClient(ClientID id, const std::string& key = "");
  IRC::Numeric removeClient(ClientID id);
  IRC::Numeric kickClient(ClientID requesterID, ClientID targetID);
  // bool hasClient(const std::string& nick) const;
  // IRC::Numeric setClientOp(const std::string& requesterNick,
  //                          const std::string& targetNick);
  // IRC::Numeric unsetClientOp(const std::string& requesterNick,
  //                            const std::string& targetNick);
  // bool isClientOp(const std::string& nick) const;
  bool hasClient(ClientID id) const;
  IRC::Numeric setClientOp(ClientID requesterID, ClientID targetID);
  IRC::Numeric unsetClientOp(ClientID requesterID, ClientID targetID);

  bool isClientOp(ClientID id) const;
  const std::set<ClientID> getJoinedClients() const;

  int getClientNumber() const;

  // IRC::Numeric setMode(const std::string& requesterNick, IChannelMode mode,
  //                      std::vector<std::string> params);
  // IRC::Numeric addMode(const std::string& requesterNick, IChannelMode mode,
  //                      const std::string& param);
  // IRC::Numeric removeMode(const std::string& requesterNick, IChannelMode
  // mode,
  //                         const std::string& param);
  IRC::Numeric setMode(ClientID id, IChannelMode mode,
                       std::vector<std::string> params);
  IRC::Numeric addMode(ClientID id, IChannelMode mode,
                       const std::string& param);
  IRC::Numeric removeMode(ClientID id, IChannelMode mode,
                          const std::string& params);
  const std::string getMode();

  // IRC::Numeric addToInviteList(const std::string& requesterNick,
  //                              const std::string& targetNick);
  IRC::Numeric addToInviteList(ClientID requesterId, ClientID targetId);
  // bool isInInviteList(const std::string& nick) const;
  bool isInInviteList(ClientID id) const;

  // IRC::Numeric setTopic(const std::string& nick, const std::string& topic);
  // IRC::Numeric reqTopic(const std::string& nick);
  IRC::Numeric setTopic(ClientID id, const std::string& topic);
  IRC::Numeric reqTopic(ClientID id);
  const std::string& getTopic() const;

  bool isLimited() const;
  int getMaxMember() const;
  bool isInviteOnly() const;
  bool isTopicOpOnly() const;
  bool hasKey() const;
  IRC::Numeric setInviteOnly(ClientID id);
  IRC::Numeric unsetInviteOnly(ClientID id);
  IRC::Numeric setTopicOpOnly(ClientID id);
  IRC::Numeric unsetTopicOpOnly(ClientID id);
  IRC::Numeric setKey(ClientID id, const std::string& newKey);
  IRC::Numeric unsetKey(ClientID id);
  IRC::Numeric setLimit(ClientID id, int limit);
  IRC::Numeric unsetLimit(ClientID id);

 private:
  std::string channelName;
  std::string topic;
  std::string key;
  IChannel::IChannelMode mode;  // inv, topic, key, limit
  size_t limit;

  std::set<ClientID> invitedUsers;
  std::set<ClientID> joinedUsers;
  std::set<ClientID> operators;
};

#endif
