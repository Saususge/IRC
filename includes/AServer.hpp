#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <poll.h>
#include <sys/poll.h>

#include <vector>

#include "IChannelResistry.hpp"
#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"

class AServer {
 public:
  virtual IClientRegistry& clients() = 0;
  virtual IChannelResistry& channels() = 0;
  virtual IServerConfig& serverConfig() = 0;

  int acceptSession();
  // Defererd disconnection
  int disconnectSession(ISession* session);

  ISession* findSession(const std::string& nick);

  int run();
  // TODO:
  // Add more behavior that Server can do

 private:
  std::vector<ISession*> _sessions;
  std::vector<struct pollfd> _pollfds;
  int _listeningSocketFD;
};
#endif