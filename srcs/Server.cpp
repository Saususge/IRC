#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _serverFd(-1), _pollFds(), _inbuf(), _outbuf(), manager() {
  initSocketOrDie();
}

Server::~Server() {
  for (size_t i = 0; i < _pollFds.size(); ++i) ::close(_pollFds[i].fd);
  _pollFds.clear();
  _inbuf.clear();
  _outbuf.clear();
}

void Server::initSocketOrDie() {
  _serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_serverFd < 0) {
    perror("socket");
    exit(1);
  }

  int opt = 1;
  if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    exit(1);
  }

  setNonBlockingOrDie(_serverFd);

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(_serverFd, SOMAXCONN) < 0) {
    perror("listen");
    exit(1);
  }

  struct pollfd pfd;
  pfd.fd = _serverFd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollFds.push_back(pfd);
}

void Server::setNonBlockingOrDie(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    perror("fcntl F_GETFL");
    exit(1);
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    perror("fcntl F_SETFL");
    exit(1);
  }
}

void Server::run() {
  std::cout << "Server running on port " << _port << "..." << std::endl;

  while (true) {
    int ret = poll(&_pollFds[0], _pollFds.size(), -1);
    if (ret < 0) {
      perror("poll");
      break;
    }

    size_t currentSize = _pollFds.size();
    for (size_t i = 0; i < currentSize; ++i) {
      if (_pollFds[i].revents == 0) continue;

      if (_pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
        closeClient(i);
        i--;
        currentSize--;
        continue;
      }

      if (_pollFds[i].revents & POLLIN) {
        if (_pollFds[i].fd == _serverFd) {
          acceptClients();
        } else {
          handleClientReadable(i);
        }
      }

      if (i < _pollFds.size() && (_pollFds[i].revents & POLLOUT)) {
        handleClientWritable(i);
      }
    }
  }
}

void Server::acceptClients() {
  struct sockaddr_in clientAddr;
  socklen_t clientLen = sizeof(clientAddr);
  int clientFd =
      accept(_serverFd, (struct sockaddr*)&clientAddr, &clientLen);

  if (clientFd < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("accept");
    }
    return;
  }

  setNonBlockingOrDie(clientFd);

  struct pollfd pfd;
  pfd.fd = clientFd;
  pfd.events = POLLIN | POLLOUT;
  pfd.revents = 0;
  _pollFds.push_back(pfd);

  _inbuf[clientFd] = "";
  _outbuf[clientFd] = "";
  std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::closeClient(size_t pollIndex) {
  int fd = _pollFds[pollIndex].fd;
  close(fd);

  _pollFds.erase(_pollFds.begin() + pollIndex);
  _inbuf.erase(fd);
  _outbuf.erase(fd);
  std::cout << "Client disconnected: " << fd << std::endl;
}

void Server::handleClientReadable(size_t pollIndex) {
  int fd = _pollFds[pollIndex].fd;
  char buf[512];
  ssize_t n = recv(fd, buf, sizeof(buf), 0);

  if (n <= 0) {
    closeClient(pollIndex);
    return;
  }

  _inbuf[fd].append(buf, n);

  size_t pos;
  while ((pos = _inbuf[fd].find("\r\n")) != std::string::npos) {
    std::string line = _inbuf[fd].substr(0, pos);
    _inbuf[fd].erase(0, pos + 2);
    onLine(fd, line);
  }
  while ((pos = _inbuf[fd].find("\n")) != std::string::npos) {
    std::string line = _inbuf[fd].substr(0, pos);
    if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
    _inbuf[fd].erase(0, pos + 1);
    onLine(fd, line);
  }
}

void Server::onLine(int fd, const std::string& line) {
  std::cout << "Received from " << fd << ": " << line << std::endl;
  manager.doRequest(line, fd);
}

void Server::queueMessage(int fd, const std::string& msg) {
  if (_outbuf.find(fd) == _outbuf.end()) return;
  _outbuf[fd] += msg;
}

void Server::handleClientWritable(size_t pollIndex) {
  int fd = _pollFds[pollIndex].fd;
  std::string& out = _outbuf[fd];

  if (out.empty()) return;

  ssize_t n = send(fd, out.c_str(), out.size(), 0);
  if (n < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      closeClient(pollIndex);
    }
    return;
  }
  out.erase(0, n);
}
