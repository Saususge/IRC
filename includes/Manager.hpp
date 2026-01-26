#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <iostream>
#include <map>
#include <sstream>

#include "Channel.hpp"
#include "Client.hpp"

class Server;

class Manager {
 public:
  Manager();
  ~Manager();

  int doRequest(Server& server, int fd, std::string request);
  void addClient(int fd, bool isPassAvaliable=true);
  void removeClient(int fd);

 private:
  std::map<int, Client> users;
  std::map<std::string, Channel> channels;

  std::string pass;
};

#endif
