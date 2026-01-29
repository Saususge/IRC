#include "Server.hpp"
#include "Response.hpp" // 응답 생성용
#include <iostream>

Server::Server(int port, const std::string& password) 
    : _config(password) {
    initSocket(port); 
}

Server::~Server() {}

const IServerConfig& Server::serverConfig() const {
    return _config;
}

void Server::onClientConnected(int fd) {
    if (_users.find(fd) == _users.end()) {
        _users[fd] = Client(); 
        std::cout << "Client connected: " << fd << std::endl;
    }
}

void Server::onClientDisconnected(int fd) {
    if (_users.find(fd) != _users.end()) {
        // TODO: leave channels, notify others, etc.
        _users.erase(fd);
        std::cout << "Client disconnected: " << fd << std::endl;
    }
}

void Server::onClientMessage(int fd, const std::string& message) {
    std::cout << "Received from " << fd << ": " << message << std::endl;
        dispatchCommand(fd, message);
}

void Server::dispatchCommand(int fd, const std::string& commandLine) {
  // Todo: parsing and executing commands
  // test echo
  ISession* session = findSession(fd);
  if (session) {
      session->send("Echo: " + commandLine + "\r\n");
  }
}