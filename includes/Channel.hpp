#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <unordered_map>

struct MemberInfo {
  int fd;
  bool voice;  // unnessary variable for ft-irc
  bool op;
};

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

  std::unordered_map<std::string, MemberInfo> users;

 public:
  Channel();
  ~Channel();

  void addUser(std::string nickname);
  void delUser(std::string nickname);
};

#endif
