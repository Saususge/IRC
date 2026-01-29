#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <vector>
#include <map>
#include <sys/poll.h>
#include <string>

#include "ISession.hpp"
#include "IServerConfig.hpp"

// Forward Declaration
class ISession;

class AServer {
 public:
  virtual ~AServer();

  void run();

  virtual const IServerConfig& serverConfig() const = 0;

 protected:
  virtual void onClientConnected(int fd) = 0;
  virtual void onClientDisconnected(int fd) = 0;
  virtual void onClientMessage(int fd, const std::string& message) = 0;

  void initSocket(int port);
  ISession* findSession(int fd);

 protected:
  int _listeningSocketFD;
  std::vector<struct pollfd> _pollfds;
  std::map<int, ISession*> _sessions;

 private:
  void acceptClient();
  void handlePollIn(size_t index);
  void handlePollOut(size_t index);
  void processDeletionQueue();
};

#endif