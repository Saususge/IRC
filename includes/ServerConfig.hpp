#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>

#include "IServerConfig.hpp"

class ServerConfig : public IServerConfig {
 public:
  ServerConfig(const std::string& password, const std::string& creationDate,
               const std::string& serverName = "ft_irc_server",
               const std::string& version = "1.0.0")
      : _password(password),
        _creationDate(creationDate),
        _serverName(serverName),
        _version(version) {}

  const std::string& getPassword() const { return _password; }
  const std::string& getServerName() const { return _serverName; }
  const std::string& getVersion() const { return _version; }
  const std::string& getCreationDate() const { return _creationDate; }
  const std::string getUserModes() const { return "n/a"; }
  const std::string getChannelModes() const { return "itkol"; }

 private:
  const std::string _password;
  const std::string _serverName;
  const std::string _version;
  const std::string _creationDate;
};
#endif