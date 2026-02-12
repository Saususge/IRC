#include "Server.hpp"

#include <ctime>
#include <iostream>

#include "ClientManagement.hpp"
#include "Command.hpp"
#include "ServerConfig.hpp"
#include "SessionManagement.hpp"

static std::string currentTimeString() {
  time_t t = std::time(NULL);
  const char* s = std::ctime(&t);

  if (!s) return std::string();

  std::string str(s);
  return (str.empty() || str[str.length() - 1] != '\n')
             ? str
             : str.erase(str.length() - 1);
}

static std::string strToUpper(const std::string& str) {
  std::string newStr = str;
  for (size_t i = 0; i < str.size(); i++) {
    newStr[i] = std::toupper(str[i]);
  }
  return newStr;
}

static int fillCtx(CommandContext& ctx, std::string msg) {
  size_t last = msg.find_last_not_of("\r\n ");
  if (last == std::string::npos) return 0;
  msg.erase(last + 1);

  size_t current = 0;

  if (msg[current] == ':') {
    current = msg.find(' ');
    if (current == std::string::npos) return 0;
    current = msg.find_first_not_of(' ', current);
    if (current == std::string::npos) return 0;
  }

  size_t next_space = msg.find(' ', current);
  std::string command = msg.substr(current, next_space - current);
  ctx.setCommandType(strToUpper(command));

  if (next_space == std::string::npos) return 1;
  current = next_space;

  std::vector<std::string> args;
  while (current != std::string::npos) {
    current = msg.find_first_not_of(' ', current);
    if (current == std::string::npos) break;

    if (msg[current] == ':') {
      args.push_back(msg.substr(current + 1));
      break;
    }

    next_space = msg.find(' ', current);
    if (next_space == std::string::npos) {
      args.push_back(msg.substr(current));
      break;
    }
    args.push_back(msg.substr(current, next_space - current));
    current = next_space;
  }
  ctx.setArgs(args);
  return 1;
}

Server::Server(int port, const std::string& password) : AServer(port) {
  // Implementation of registries can be added here
  ServerConfig::setPassword(password);
  ServerConfig::setCreationDate(currentTimeString());
}

Server::~Server() {}

// TODO: remove unused fields of Server and CommandContext classes.
void Server::onClientMessage(int fd, const std::string& msg) {
  std::cout << "[Server] Msg from " << fd << ": " << msg << std::endl;

  // TODO: Command Pattern applyation
  ISession* session = SessionManagement::getSession(fd);
  if (session == NULL) return;
  ISession& sessionRef = *session;

  IClient* client = ClientManagement::getClient(session);
  if (client == NULL) return;
  IClient& clientRef = *client;

  CommandContext ctx(sessionRef, clientRef);

  if (fillCtx(ctx, msg) == 0) return;

  CommandFactory::getCommand(ctx.getCommandType()).execute(ctx);
  // pass the session pointer to match CommandContext's constructor signature
}

ClientID Server::createClient() { return ClientManagement::createClient(); }