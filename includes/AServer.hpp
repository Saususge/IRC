#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include <string>
#include <set>

#include "ISession.hpp"
#include "IServerConfig.hpp"
#include "IClientRegistry.hpp"
#include "IChannelRegistry.hpp"

class AServer {
 public:
  virtual ~AServer();

  virtual IClientRegistry& clients() = 0;
  virtual IChannelRegistry& channels() = 0;
  virtual const IServerConfig& serverConfig() const = 0;

  void run();

 protected:
  AServer(int port);

  virtual void onClientMessage(int fd, const std::string& msg) = 0;
  
  virtual void onClientDisconnect(int fd);

 private:
  void initSocketOrDie(int port);
  void acceptClient();
  void handlePollIn(size_t index);
  void handlePollOut(size_t index);

  std::set<int> removePollFDs;
  std::map<int, ISession*> _sessions;
  std::vector<struct pollfd> _pollfds;
  int _listeningSocketFD;
};

#endif