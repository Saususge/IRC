#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <iostream>
#include <map>
#include <sstream>

#include "Channel.hpp"
#include "Client.hpp"

class Manager {
 public:
  Manager();
  ~Manager();

  int doRequest(std::string request, int fd);

 private:
  std::map<int, Client> users;
  std::map<int, std::string> unregistered;
  std::map<std::string, Channel> channels;

  std::string pass;
};

#endif
