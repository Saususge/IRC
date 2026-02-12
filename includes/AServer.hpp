#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <poll.h>

#include <set>
#include <string>
#include <vector>

#include "ISession.hpp"

class AServer {
 public:
  virtual ~AServer();

  // virtual IClientRegistry& clients() = 0;
  // virtual IChannelRegistry& channels() = 0;
  // virtual const IServerConfig& serverConfig() const = 0;

  void run();

 protected:
  AServer(int port);

  virtual ISession* createSession(int fd, ClientID id);
  virtual ClientID createClient() = 0;

  virtual void onClientMessage(int fd, const std::string& msg) = 0;

 private:
  AServer(const AServer&);
  AServer& operator=(const AServer&);

  void initSocketOrDie(int port);
  void acceptClient();
  bool handlePollIn(size_t index, std::set<ISession*>& hasBytes);
  bool handlePollOut(size_t index);

  std::vector<struct pollfd> _pollfds;
  int _listeningSocketFD;
};

#endif