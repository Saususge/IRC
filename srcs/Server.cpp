#include "Server.hpp"

#include <ctime>
#include <iostream>

#include "ServerConfig.hpp"

static std::string currentTimeString() {
  time_t t = std::time(NULL);
  const char* s = std::ctime(&t);
  return s ? std::string(s) : std::string();
}

Server::Server(int port, const std::string& password)
    : AServer(port), _config(password, currentTimeString()) {
  // Implementation of registries can be added here
}

Server::~Server() {}
const IServerConfig& Server::serverConfig() const { return _config; }

void Server::onClientMessage(int fd, const std::string& msg) {
  std::cout << "[Server] Msg from " << fd << ": " << msg << std::endl;

  // TODO: Command Pattern applyation
}

void Server::onClientDisconnect(int fd) {
  // Todo: client remove at registries
  AServer::onClientDisconnect(fd);
}