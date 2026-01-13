#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

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

  int addUser(std::string nickname, int fd);
  int delUser(std::string nickname);
  int getUserInfo(std::string nickname, MemberInfo* info);
  const std::unordered_map<std::string, MemberInfo>& getUsers() const;

  int promoteToOp(std::string prompter, std::string target);

  void setInvite(bool value);
  bool getInvite() const;
  void inviteUser(std::string nickname);

  void setTopicMode(bool value);
  bool getTopicMode() const;
  void setTopic(std::string topic);
  std::string getTopic() const;

  void setKey(std::string newKey);
  std::string getKey() const;

  // in rfc 2812 section 3.2.3 `MODE #42 -k oulu` is the
  // command to remove the "oulu" channel key on channel #42
  // but in ngircd it does not need to remove key.
  void removeKey(std::string currentKey);
};

#endif
