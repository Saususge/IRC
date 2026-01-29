#ifndef SERVER_HPP
#define SERVER_HPP

#include "AServer.hpp"
#include "ServerConfig.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include <map>

class Server : public AServer {
 public:
  Server(int port, const std::string& password);
  virtual ~Server();

  virtual const IServerConfig& serverConfig() const;

 protected:
  virtual void onClientConnected(int fd);
  virtual void onClientDisconnected(int fd);
  virtual void onClientMessage(int fd, const std::string& message);

 private:
  ServerConfig _config;
  // Data Management
  std::map<int, Client> _users;
  std::map<std::string, Channel> _channels;
  void dispatchCommand(int fd, const std::string& commandLine);
};

#endif