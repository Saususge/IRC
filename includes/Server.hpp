#ifndef SERVER_HPP
#define SERVER_HPP

#include "AServer.hpp"
class Server : public AServer {
 public:
  Server(int port, const std::string& password);
  virtual ~Server();

 protected:
  virtual void onClientMessage(int fd, const std::string& msg);
  virtual ClientID createClient();
};

#endif