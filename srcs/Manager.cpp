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
    if (tok[0] == ':') {
      std::string trailing;
      std::getline(ss, trailing);
      tok = tok.substr(1) + trailing;  // ':' 제거하고 나머지 이어붙임
      tokVec.push_back(tok);
      std::cout << "doRequest token: " << tok << "$" << std::endl;
      break;  // trailing parameter는 항상 마지막이므로 루프 종료
    } else {
      tokVec.push_back(tok);
      std::cout << "doRequest token: " << tok << "$" << std::endl;
    }
  }

  std::string cmd = strToLower(getCommand(tokVec));

  // refactor to switch-case later
  if (cmd == "nick") {
    std::cout << "nick" << std::endl;

    for (std::map<int, std::string>::iterator iter = unregistered.begin();
         iter != unregistered.end(); iter++) {
      if (iter->second == tokVec[1]) {
#ifdef DEBUG
        std::cerr << "The nickname " << tokVec[1] << " is already exist."
                  << std::endl;
#endif

        return -1;
      }
    }

    for (std::map<int, Client>::iterator iter = users.begin();
         iter != users.end(); iter++) {
      if (iter->second.getNickname() == tokVec[1]) {
#ifdef DEBUG
        std::cerr << "The nickname " << tokVec[1] << " is already exist."
                  << std::endl;
#endif

        return -1;
      }
    }

    unregistered.insert(std::pair<int, std::string>(fd, tokVec[1]));
  } else if (cmd == "user") {
    std::cout << "user" << std::endl;
    if (unregistered.find(fd) == unregistered.end()) {
#ifdef DEBUG
      std::cerr << "The client " << fd << " does not send user request."
                << std::endl;
#endif
      return -1;
    }

    users.insert(
        std::pair<int, Client>(fd, Client(unregistered[fd], tokVec[1], tokVec[4])));
    unregistered.erase(fd);
  } else if (cmd == "pass") {
    std::cout << "pass" << std::endl;
  } else if (cmd == "join") {
    std::cout << "join" << std::endl;
    if (channels.find(tokVec[1]) == channels.end()) {
      channels.insert(std::pair<std::string, Channel>(tokVec[1], Channel()));
    }

    Channel& channel = channels.find(tokVec[1])->second;

    return channel.addUser(fd, &(users.find(fd)->second));

  } else if (cmd == "part") {
    std::cout << "part" << std::endl;
    std::map<std::string, Channel>::iterator iter = channels.find(tokVec[1]);
    if (iter == channels.end()) return -1; // ERR_NOSUCHCHANNEL

    return iter->second.delUser(users.find(fd)->second);

  }

  return 1;
}
