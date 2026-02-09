#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "IClient.hpp"
#include "defs.hpp"

class Client : public IClient {
 public:
  Client(ClientID id);
  ~Client();

  // PASS
  IRC::Numeric Authenticate();
  // NICK
  IRC::Numeric setNick(const std::string& nick);
  // USER
  IRC::Numeric setUserInfo(const std::string& user,
                           const std::string& realName);
  const std::string& getNick() const;
  const std::string& getUser() const;
  const std::string& getRealName() const;

  void setSessionID(SessionID id);
  SessionID getSessionID() const;

  ClientID getID() const;

  void Register();
  bool isRegistered() const;
  bool isAuthenticated() const;

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;

  short _loginFlags;
  bool _registered;

  IRC::Numeric checkLoginFlags();

  SessionID _sessionID;
  ClientID _clientID;
};

#endif
