#ifndef ISESSION_HPP
#define ISESSION_HPP

#include <string>

class ISession {
 public:
  virtual ~ISession() {};

  virtual int send(const std::string& msg) = 0;
  // Deferred Close
  virtual int disconnect() = 0;
  // Needed for deferred close pollFDs in AServer
  virtual int getSocketFD() const = 0;
};

#endif