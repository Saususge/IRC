#include "Server.hpp"

#include <iostream>

Server::Server(int port, const std::string& password)
    : AServer(port), _config(password) {
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