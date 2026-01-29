#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>

#include "IServerConfig.hpp"

class ServerConfig : public IServerConfig {
 public:
  ServerConfig(const std::string& password) : _password(password) {}

  const std::string& getPassword() const { return _password; }

 private:
  const std::string _password;
};
#endif