#include "AServer.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Session.hpp"
#include "utils.hpp"

AServer::AServer(int port) { initSocketOrDie(port); }

AServer::~AServer() {
  for (std::map<int, ISession*>::iterator it = _sessions.begin();
       it != _sessions.end(); ++it) {
    delete it->second;
  }
  _sessions.clear();
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
    std::set<int> removePollFDs;
    int ret = poll(&_pollfds[0], _pollfds.size(), -1);
    if (ret < 0) break;

    for (size_t i = 0; i < _pollfds.size(); ++i) {
      if (_pollfds[i].revents == 0) continue;

      if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
        if (POLLHUP) removePollFDs.insert(_pollfds[i].fd);
        onClientDisconnect(_pollfds[i].fd);
        continue;
      }

      if (_pollfds[i].revents & POLLIN) {
        if (_pollfds[i].fd == _listeningSocketFD)
          acceptClient();
        else
          handlePollIn(i);
      }
    }
  }
}

void AServer::acceptClient() {
  // todo: accept logic
}

void AServer::handlePollIn(size_t index) {
  int fd = _pollfds[index].fd;
  ISession* session = _sessions[fd];

  std::string msg = session->read();

  if (!msg.empty()) {
    this->onClientMessage(fd, msg);
  }
}

void AServer::onClientDisconnect(int fd) {
  if (_sessions.find(fd) != _sessions.end()) {
    delete _sessions[fd];
    _sessions.erase(fd);
  }
}
