#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <iostream>
#include <map>
#include <sstream>

#include "Channel.hpp"

class Manager {
 public:
  Manager();
  ~Manager();

  int doRequest(std::string request, int fd);

 private:
  std::map<int, User> users;
  std::map<std::string, Channel> channels;
};

#endif
