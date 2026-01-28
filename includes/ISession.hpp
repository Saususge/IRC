#ifndef ISESSION_HPP
#define ISESSION_HPP

#include <string>

class ISession {
 public:
  virtual ~ISession() {};

  virtual int send(const std::string& msg) = 0;
  virtual int disconnect() = 0;  // Deferred Close

  virtual int getSocketFD() = 0;  // Needed for deferred close
};

#endif