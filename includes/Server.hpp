#ifndef SERVER_HPP
#define SERVER_HPP

#include "AServer.hpp"
#include "ServerConfig.hpp"

class Server : public AServer {
 public:
  Server(int port, const std::string& password);
  virtual ~Server();

  virtual IClientRegistry& clients();
  virtual IChannelRegistry& channels();
  virtual const IServerConfig& serverConfig() const;

 protected:
  virtual void onClientMessage(int fd, const std::string& msg);
  virtual void onClientDisconnect(int fd);

 private:
  ServerConfig _config;
};

#endif