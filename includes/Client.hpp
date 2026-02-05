#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "IClient.hpp"
#include "defs.hpp"

class Client : public IClient {
 public:
  Client();
  ~Client();

  // PASS
  IRC::Numeric Authenticate(const std::string& password);
  // NICK
  IRC::Numeric setNick(const std::string& nick);
  // USER
  IRC::Numeric setUserInfo(const std::string& user,
                           const std::string& realName);
  const std::string& getNick() const;
  const std::string& getUser() const;
  const std::string& getRealName() const;

  SessionID getSessionID() const;

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;

  short loginFlags;
  bool registered;

  IRC::Numeric checkLoginFlags();

  SessionID _sessionID;
};

#endif
