#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

namespace Response {
std::string build(const std::string& code, const std::string& target,
                  const std::string& message) {
  return ":" + std::string("irc.local") + " " + code + " " + target + " " +
         message + "\r\n";
}

// Alias of `build`
inline std::string error(const std::string& code, const std::string& target,
                         const std::string& message) {
  return build(code, target, message);
}
};  // namespace Response

#endif
