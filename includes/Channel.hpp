#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "numeric.hpp"

#define DEBUG

struct User {
  // other users see in channels. unique on the network, can change while
  // connected.
  std::string nickname;

  // idnet. comes from the client's connection/login; servers use it to identify
  // the account or system user behind the session and is part of the prefix
  // nick!username@host
  std::string username;

  // purely descriptive metadata that apperars in WHOIS output.
  std::string realname;

  std::vector<std::string> joinedChannels;

  User(std::string nickname, std::string username, std::string realname)
      : nickname(nickname), username(username), realname(realname) {}
};

struct MemberInfo {
  int fd;
  User* user;
  bool voice;  // unnessary variable for ft-irc
  bool op;

  MemberInfo() : user(NULL) {}
  MemberInfo(int fd, User* user, bool op)
      : fd(fd), user(user), voice(false), op(op) {}
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

  int addUser(int fd, User* user, bool isCreator = false, std::string key = "");
  int delUser(User user);
  int getUserInfo(std::string nickname, MemberInfo* info = NULL);
  const std::map<std::string, MemberInfo>& getUsers() const;

  // MODE <chan> +o <nick>
  int promoteToOp(User user, std::string targetNick);

  int setInvite(User user, bool value);
  bool getInvite() const;
  int inviteUser(User user, std::string targetNick);

  int setTopicMode(User user, bool value);
  bool getTopicMode() const;
  int setTopic(User user, std::string topic);
  std::string getTopic() const;

  int setKey(User user, std::string newKey);
  std::string getKey() const;

  int setUserLimit(User user, int newLimit);
  int getUserLimit() const;

  // in rfc 2812 section 3.2.3 `MODE #42 -k oulu` is the
  // command to remove the "oulu" channel key on channel #42
  // but in ngircd it does not need to remove key.
  void removeKey(std::string currentKey);
};

#endif
