#ifndef ICLIENT_HPP
#define ICLIENT_HPP

#include <string>
#include <vector>

class IClient {
 public:
  virtual ~IClient() {};

  virtual void onAuthenticated() = 0;
  virtual bool IsAuthenticated() = 0;
  virtual void onRegistered() = 0;
  virtual bool isRegistered() = 0;

  virtual bool hasNick() = 0;
  virtual void setNick(const std::string& nick) = 0;
  virtual bool hasUserInfo() = 0;
  virtual bool setUserInfo(const std::string& user,
                           const std::string& realName) = 0;

  virtual int send(const std::string& msg) = 0;

  virtual int joinChannel(const std::string& channelName) = 0;
  virtual int partChannel(const std::string& channelName) = 0;
  virtual std::vector<const std::string&> getJoinedChannels() = 0;
};
#endif  // ICLIENT_HPP