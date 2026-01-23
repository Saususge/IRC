#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include "numeric.hpp"

class Response {
 public:
  static std::string build(const std::string& code, const std::string& target, const std::string& message) {
    return ":" + std::string("irc.local") + " " + code + " " + target + " " + message + "\r\n";
  }
  
  static std::string error(const std::string& code, const std::string& target, const std::string& message) {
     return build(code, target, message);
  }
};

#endif
