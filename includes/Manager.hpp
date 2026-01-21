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

  std::string getCommand(std::vector<std::string>& tokens) {
    if (tokens[0][0] != ':') return tokens[0];
    return tokens[1];
  }

  std::string strToLower(std::string str) {
    for (std::string::iterator iter = str.begin(); iter < str.end(); iter++) {
      *iter = static_cast<char>(std::tolower(*iter));
    }
    return (str);
  }

  int doRequest(std::string request, int fd) {
    std::stringstream ss(request);
    std::string tok;
    std::vector<std::string> tokVec;

    while (ss >> tok) {
      if (tok[0] == ':') std::getline(ss, tok);

      std::cout << "doRequest token: " << tok << std::endl;
      tokVec.push_back(tok);
    }

    std::string cmd = strToLower(getCommand(tokVec));

    if (cmd == "nick") {
    } else if (cmd == "user") {
    } else if (cmd == "pass") {
    } else if (cmd == "join") {
      if (channels.find(tokVec[1]) == channels.end()) {
        channels.emplace(tokVec[1], Channel());
      }

      Channel channel = channels.find(tokVec[1])->second;

      channel.addUser(fd, users.find(fd)->second);
    }
  }

 private:
  std::unordered_map<int, User> users;
  std::unordered_map<std::string, Channel> channels;
};

#endif
