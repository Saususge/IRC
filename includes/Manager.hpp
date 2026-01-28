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

  void sendWelcomeMsg(Server& server, int fd, std::string nickname);
  void sendPassMismatch(Server& server, int fd, std::string nickname);

  bool isValidParam(Server& server, int fd, size_t paramNum, std::vector<std::string>& tokVec);
  bool isAlreadyRegisterd(Server& server, int fd);
  bool isNotRegisterd(Server& server, int fd);

  int getFdByNick(Server& server, int fd, std::string callerNick, std::string target);

};

#endif
