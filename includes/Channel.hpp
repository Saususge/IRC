#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Client.hpp"

#define DEBUG

// User struct removed, replaced by Client class in Client.hpp

struct MemberInfo {
  int fd;
  Client* client;
  bool voice;  // unnessary variable for ft-irc
  bool op;

  MemberInfo() : client(NULL) {}
  MemberInfo(int fd, Client* client, bool op)
      : fd(fd), client(client), voice(false), op(op) {}
};

typedef std::map<std::string, MemberInfo>::iterator mIter;

class Channel {
 private:
  // i(invite only), t(restrictions of topic), k(channel key),
  // o(channel operator), l(user limit)
  bool inviteOnly;
  std::set<std::string> invitedUsers;

  bool restrictTopic;
  std::string topic;

  std::string key;
  int userLimit;

  std::map<std::string, MemberInfo> users;

 public:
  Channel();
  ~Channel();

  int addUser(int fd, Client* client, bool isCreator = false);
  int delUser(Client* client);
  int getUserInfo(std::string nickname, MemberInfo* info = NULL);
  const std::map<std::string, MemberInfo>& getUsers() const;

  // OPER <name> <password>
  int promoteToOp(Client* client);
  // MODE <chan> +o <nick>
  int promoteToOp(Client* client, std::string targetNick);

  int setInvite(Client* client, bool value);
  bool getInvite() const;
  int inviteUser(Client* client, std::string targetNick);

  void setTopicMode(Client* client, bool value);
  bool getTopicMode() const;
  void setTopic(Client* client, std::string topic);
  std::string getTopic() const;

  void setKey(Client* client, std::string newKey);
  std::string getKey() const;

  // in rfc 2812 section 3.2.3 `MODE #42 -k oulu` is the
  // command to remove the "oulu" channel key on channel #42
  // but in ngircd it does not need to remove key.
  void removeKey(std::string currentKey);
};

#endif
