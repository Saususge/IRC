#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {
 public:
  Client(std::string nickname, std::string username, std::string realname);
  ~Client();

  std::string getNickname() const;
  void setNickname(std::string nickname);

  std::string getUsername() const;
  
  std::string getRealname() const;

  // 채널 관련 메서드는 필요시 추가
  void joinChannel(std::string channelName);
  void leaveChannel(std::string channelName);

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;
  
  std::vector<std::string> _joinedChannels;
};

#endif
