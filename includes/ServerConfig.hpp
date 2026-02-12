#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>

namespace ServerConfig {
void setPassword(const std::string& pw);
const std::string& getPassword();
const std::string& getServerName();
const std::string& getVersion();
void setCreationDate(const std::string& date);
const std::string& getCreationDate();
const std::string& getUserModes();
const std::string& getChannelModes();
}  // namespace ServerConfig
#endif