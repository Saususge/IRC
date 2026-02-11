#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

#include "numeric.hpp"

namespace Response {
inline std::string build(const std::string& code, const std::string& target,
                         const std::string& message) {
  return ":" + std::string("irc.local") + " " + code + " " + target + " " +
         message + "\r\n";
}

inline std::string build(IRC::Numeric numeric, const std::string& target,
                         const std::string& message) {
  return build(IRC::numericToCode(numeric), target, message);
}

// Alias of `build`
inline std::string error(const std::string& code, const std::string& target,
                         const std::string& message) {
  return build(code, target, message);
}

inline std::string error(IRC::Numeric numeric, const std::string& target,
                         const std::string& message) {
  return build(IRC::numericToCode(numeric), target, message);
}
};  // namespace Response

#endif
