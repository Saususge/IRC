#ifndef ISESSION_HPP
#define ISESSION_HPP

#include <string>

#include "defs.hpp"

class ISession {
 public:
  enum SessionStatus { ALIVE, CLOSING, DEAD };

  virtual ~ISession() {};

  virtual int read() = 0;
  virtual std::string readLine() = 0;

  virtual void enqueueMsg(const std::string& msg) = 0;
  virtual int send() = 0;
  virtual int getSocketFD() const = 0;

  virtual void setID(SessionID id) = 0;
  virtual SessionID getID() const = 0;

  virtual void setClientID(ClientID id) = 0;
  virtual ClientID getClientID() const = 0;

  virtual void setStatus(SessionStatus status) = 0;
  virtual SessionStatus getStatus() = 0;

  virtual bool isOutBufEmpty() = 0;
};

#endif