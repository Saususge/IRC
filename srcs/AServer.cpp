#include "AServer.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <iostream>
#include <set>

#include "ClientManagement.hpp"
#include "Session.hpp"
#include "SessionManagement.hpp"
#include "utils.hpp"

extern sig_atomic_t running;

AServer::AServer(int port) : _listeningSocketFD(-1) { initSocketOrDie(port); }

AServer::~AServer() {
  SessionManagement::deleteScheduledSession();
  if (_listeningSocketFD != -1) close(_listeningSocketFD);
}

void AServer::initSocketOrDie(int port) {
  _listeningSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (_listeningSocketFD < 0) exit_with_error("socket failed");

  int opt = 1;
  setsockopt(_listeningSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  fcntl(_listeningSocketFD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(_listeningSocketFD, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    exit_with_error("bind failed");
  if (listen(_listeningSocketFD, SOMAXCONN) < 0)
    exit_with_error("listen failed");

  struct pollfd pfd;
  pfd.fd = _listeningSocketFD;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollfds.push_back(pfd);
}

void AServer::run() {
  std::cout << "Server started..." << std::endl;
  while (running) {
    std::set<int> removeFDs;
    std::set<ISession*> hasBytes;
    int ret = poll(&_pollfds[0], _pollfds.size(), -1);
    if (ret < 0) break;

    for (size_t i = 0; i < _pollfds.size(); ++i) {
      if (_pollfds[i].revents == 0) continue;

      if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
        removeFDs.insert(_pollfds[i].fd);
        SessionManagement::scheduleForDeletion(_pollfds[i].fd, ISession::DEAD);
        continue;
      }
      bool shouldRemove = false;
      if (_pollfds[i].revents & POLLIN) {
        if (_pollfds[i].fd == _listeningSocketFD)
          acceptClient();
        else {
          shouldRemove = handlePollIn(i, hasBytes);
        }
      } else if (_pollfds[i].revents & POLLOUT) {
        shouldRemove = handlePollOut(i);
      }
      if (shouldRemove) removeFDs.insert(_pollfds[i].fd);
    }

    for (std::set<ISession*>::iterator iter = hasBytes.begin();
         iter != hasBytes.end(); iter++) {
      std::string line = (*iter)->readLine();
      while (!(line.empty())) {
        this->onClientMessage((*iter)->getSocketFD(), line);
        line = (*iter)->readLine();
      }
    }

    const std::set<int> releasedFDs =
        SessionManagement::deleteScheduledSession();
    for (std::set<int>::iterator it = releasedFDs.begin();
         it != releasedFDs.end(); ++it) {
      int fd = *it;
      for (std::vector<struct pollfd>::iterator pIt = _pollfds.begin();
           pIt != _pollfds.end(); ++pIt) {
        if (pIt->fd == fd) {
          _pollfds.erase(pIt);
          break;
        }
      }
    }
  }
}

void AServer::acceptClient() {
  int clientFD = accept(_listeningSocketFD, NULL, NULL);
  if (clientFD < 0) return;

  fcntl(clientFD, F_SETFL, O_NONBLOCK);

  struct pollfd pfd;
  pfd.fd = clientFD;
  pfd.events = POLLIN | POLLOUT;
  pfd.revents = 0;
  _pollfds.push_back(pfd);

  ClientID id = createClient();

  ISession* session = createSession(clientFD, id);
  session->setClientID(id);
  SessionManagement::addSession(session);
  IClient* client = ClientManagement::getClient(id);
  if (client == NULL) assert(0 && "client not found that just created. wtf?");
  client->setSessionID(session->getID());
  std::cout << "Client connected: fd=" << clientFD << std::endl;
}

ISession* AServer::createSession(int fd, ClientID id) {
  return new Session(fd, id);
}

bool AServer::handlePollIn(size_t index, std::set<ISession*>& hasBytes) {
  int fd = _pollfds[index].fd;
  ISession* session = SessionManagement::getSession(fd);
  if (session == NULL) {
    return true;
  }
  int msg = session->read();
  if (msg == 0) {
    return true;
  }
  hasBytes.insert(session);
  return false;
}

bool AServer::handlePollOut(size_t index) {
  int fd = _pollfds[index].fd;
  ISession* session = SessionManagement::getSession(fd);
  if (session == NULL) {
    return true;
  }

  int retVal = session->send();
  if (retVal == 1) return true;
  return false;
}
