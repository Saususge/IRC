#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <poll.h>

#include <string>
#include <vector>

#include "IChannelRegistry.hpp"
#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"

class AServer {
 public:
  virtual ~AServer();

  virtual IClientRegistry& clients() = 0;
  virtual IChannelRegistry& channels() = 0;
  virtual const IServerConfig& serverConfig() const = 0;

  void run();

 protected:
  AServer(int port);

  virtual ISession* createSession(int fd);

  virtual void onClientMessage(int fd, const std::string& msg) = 0;

 private:
  AServer(const AServer&);
  AServer& operator=(const AServer&);

  void initSocketOrDie(int port);
  void acceptClient();
  bool handlePollIn(size_t index);

  std::vector<struct pollfd> _pollfds;
  int _listeningSocketFD;
};

#endif