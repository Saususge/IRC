#ifndef ISESSION_HPP
#define ISESSION_HPP

#include <string>

class ISession {
 public:
  virtual ~ISession() {};

  virtual std::string read() = 0;

  virtual int send(const std::string& msg) = 0;
  virtual int getSocketFD() const = 0;
};

#endif