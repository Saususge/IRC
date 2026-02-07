#ifndef ISESSION_HPP
#define ISESSION_HPP

#include <string>

#include "defs.hpp"

class ISession {
 public:
  virtual ~ISession() {};

  virtual std::string read() = 0;

  virtual int enqueueMsg(const std::string& msg) = 0;
  virtual int send() = 0;
  virtual int getSocketFD() const = 0;
  virtual SessionID getID() const = 0;
  virtual ClientID getClientID() const = 0;
};

#endif