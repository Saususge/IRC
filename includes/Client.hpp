#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {
 public:
  Client();
  Client(std::string nickname, std::string username, std::string realname);
  ~Client();

  std::string getNickname() const;
  void setNickname(std::string nickname);

  std::string getUsername() const;
  
  std::string getRealname() const;

  void onPass();
  void onNick();
  void onUser();

  bool getPass() const;
  bool getNick() const;
  bool getUser() const;
  short getLoginFlags() const;

  bool isRegistrable() const;

  bool getAuthorized() const;
  void setAuthorized(bool value);

  // 채널 관련 메서드는 필요시 추가
  void joinChannel(std::string channelName);
  void leaveChannel(std::string channelName);

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;

  short loginFlags;
  bool authorized;
  
  std::vector<std::string> _joinedChannels;
};

#endif
