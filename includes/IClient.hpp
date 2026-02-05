#ifndef ICLIENT_HPP
#define ICLIENT_HPP

#include <string>

#include "numeric.hpp"

class IClient {
 public:
  typedef int ClientFlag;

  static const ClientFlag RPASS = 0b100;
  static const ClientFlag RNICK = 0b010;
  static const ClientFlag RUSER = 0b001;

  virtual ~IClient() {};

  // Pass
  virtual IRC::Numeric Authenticate(const std::string& password) = 0;
  // NICK
  virtual IRC::Numeric setNick(const std::string& nick) = 0;
  // USER
  virtual IRC::Numeric setUserInfo(const std::string& user,
                                   const std::string& realName) = 0;
  virtual const std::string& getNick() const = 0;
  virtual const std::string& getUser() const = 0;
  virtual const std::string& getRealName() const = 0;
};
#endif  // ICLIENT_HPP