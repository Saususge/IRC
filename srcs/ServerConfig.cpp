#include "ServerConfig.hpp"

#include <string>

namespace {
std::string _password;
std::string _serverName = "irc.local";
std::string _version = "1.0.0";
std::string _userModes = "n/a";
std::string _channelModes = "itkol";
std::string _creationDate;
}  // namespace

namespace ServerConfig {
void setPassword(const std::string& pw) { _password = pw; }
const std::string& getPassword() { return _password; }
const std::string& getServerName() { return _serverName; }
const std::string& getVersion() { return _version; }
void setCreationDate(const std::string& date) { _creationDate = date; }
const std::string& getCreationDate() { return _creationDate; }
const std::string& getUserModes() { return _userModes; }
const std::string& getChannelModes() { return _channelModes; }
}  // namespace ServerConfig