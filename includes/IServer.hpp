#ifndef ISERVER_HPP
#define ISERVER_HPP

#include "IChannelResistry.hpp"
#include "IClientRegistry.hpp"
#include "ISession.hpp"

class IServer {
 public:
  virtual ~IServer() {};

  virtual IClientRegistry& clients() = 0;

  virtual IChannelResistry& channels() = 0;

  virtual int acceptSession() = 0;
  // Defererd disconnection
  virtual int disconnectSession(ISession* session) = 0;
  // TODO:
  // IServer may need IDisconnector for IContext
  // Add more behavior that Server can do
  virtual int run() = 0;
};
#endif