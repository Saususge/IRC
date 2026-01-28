#ifndef ISERVERCONFIG_HPP
#define ISERVERCONFIG_HPP

#include <string>

class IServerConfig {
  virtual ~IServerConfig() {};

  virtual const std::string& getPassword() const = 0;
};
#endif