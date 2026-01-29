#include "Server.hpp"
#include <iostream>

Server::Server(int port, const std::string& password) 
    : AServer(port), _config(password) {
    // Todo: Initialize registries if needed
}

Server::~Server() {}

const IServerConfig& Server::serverConfig() const { return _config; }
IClientRegistry& Server::clients() { return _clientRegistry; }
IChannelRegistry& Server::channels() { return _channelRegistry; }

void Server::onClientMessage(int fd, const std::string& msg) {
    std::cout << "[Server] Msg from " << fd << ": " << msg << std::endl;

    // TODO: Command Pattern apply here
}

void Server::onClientDisconnect(int fd) {
    // Todo: Remove client from registries
    // _clientRegistry.removeClient(fd);
    AServer::onClientDisconnect(fd);
}