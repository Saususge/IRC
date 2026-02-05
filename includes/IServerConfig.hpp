#ifndef ISERVERCONFIG_HPP
#define ISERVERCONFIG_HPP

#include <string>

class IServerConfig {
 public:
  virtual ~IServerConfig() {};

  virtual const std::string& getPassword() const = 0;
  virtual const std::string& getServerName() const = 0;
  virtual const std::string& getVersion() const = 0;
  virtual const std::string& getCreationDate() const = 0;
  virtual const std::string getUserModes() const = 0;
  virtual const std::string getChannelModes() const = 0;
};
#endif