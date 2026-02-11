#include "Parser.hpp"

// IRC message format: [:<prefix>] <command> [<params>] [:<trailing>]
Message Parser::parse(const std::string& raw) {
  Message msg;
  std::string line = raw;
  std::string::size_type pos;

  // Remove trailing \r\n
  pos = line.find("\r\n");
  if (pos != std::string::npos) {
    line = line.substr(0, pos);
  }

  // 1. Extract prefix (starts with ':')
  if (!line.empty() && line[0] == ':') {
    pos = line.find(' ');
    if (pos != std::string::npos) {
      msg.prefix = line.substr(1, pos - 1);
      line = line.substr(pos + 1);
    } else {
      msg.prefix = line.substr(1);
      return msg;
    }
  }

  // 2. Extract trailing (after " :")
  pos = line.find(" :");
  if (pos != std::string::npos) {
    msg.trailing = line.substr(pos + 2);
    line = line.substr(0, pos);
  }

  // 3. Extract command and params (space-separated)
  while (!line.empty()) {
    pos = line.find(' ');
    if (pos != std::string::npos) {
      std::string token = line.substr(0, pos);
      if (!token.empty()) {
        if (msg.command.empty()) {
          msg.command = token;
        } else {
          msg.params.push_back(token);
        }
      }
      line = line.substr(pos + 1);
    } else {
      if (msg.command.empty()) {
        msg.command = line;
      } else {
        msg.params.push_back(line);
      }
      break;
    }
  }

  return msg;
}
