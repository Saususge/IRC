#include "Manager.hpp"

/* utils for Manager class */
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
/* utils for Manager class */

Manager::Manager() {}

Manager::~Manager() {}

int Manager::doRequest(std::string request, int fd) {
  std::stringstream ss(request);
  std::string tok;
  std::vector<std::string> tokVec;

  while (ss >> tok) {
    if (tok[0] == ':') std::getline(ss, tok);

    std::cout << "doRequest token: " << tok << "$" << std::endl;
    tokVec.push_back(tok);
  }

  std::string cmd = strToLower(getCommand(tokVec));

  if (cmd == "nick") {
    std::cout << "nick" << std::endl;
  } else if (cmd == "user") {
    std::cout << "user" << std::endl;
    users.insert(std::pair<int, User>(fd, User(tokVec[1], tokVec[1], tokVec[4])));
  } else if (cmd == "pass") {
    std::cout << "pass" << std::endl;
  } else if (cmd == "join") {
    std::cout << "join" << std::endl;
    if (channels.find(tokVec[1]) == channels.end()) {
      channels.insert(std::pair<std::string, Channel>(tokVec[1], Channel()));
    }

    Channel &channel = channels.find(tokVec[1])->second;

    channel.addUser(fd, &(users.find(fd)->second));
  }
  return 1;
}
