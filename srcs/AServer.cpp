#include "AServer.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <set>

#include "Session.hpp"
#include "utils.hpp"

AServer::AServer(int port) : _listeningSocketFD(-1) { initSocketOrDie(port); }

AServer::~AServer() {
  SessionManagement::sessionReg.deleteScheduledSession();
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
  while (true) {
    std::set<int> removeFDs;
    int ret = poll(&_pollfds[0], _pollfds.size(), -1);
    if (ret < 0) break;

    for (size_t i = 0; i < _pollfds.size(); ++i) {
      if (_pollfds[i].revents == 0) continue;

      if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
        removeFDs.insert(_pollfds[i].fd);
        continue;
      }

      if (_pollfds[i].revents & POLLIN) {
        if (_pollfds[i].fd == _listeningSocketFD)
          acceptClient();
        else {
          bool shouldRemove = handlePollIn(i);
          if (shouldRemove) removeFDs.insert(_pollfds[i].fd);
        }
      }
    }

    const std::set<int> releasedFDs =
        SessionManagement::sessionReg.deleteScheduledSession();
    for (std::set<int>::iterator it = releasedFDs.begin();
         it != removeFDs.end(); ++it) {
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
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollfds.push_back(pfd);

  SessionManagement::sessionReg.addSession(createSession(clientFD));
  std::cout << "Client connected: fd=" << clientFD << std::endl;
}

ISession* AServer::createSession(int fd) { return new Session(fd); }

bool AServer::handlePollIn(size_t index) {
  int fd = _pollfds[index].fd;
  ISession* session = SessionManagement::sessionReg.getSession(fd);
  if (session == NULL) {
    return true;
  }
  std::string msg = session->read();
  if (msg.empty()) {
    return true;
  }
  this->onClientMessage(fd, msg);
  return false;
}
