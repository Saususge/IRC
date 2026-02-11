#include "Command.hpp"

CommandContext::CommandContext(ISession& sessionRef, IClient& clientRef,
                               const IServerConfig& serverConfig)
    : sessionRef(sessionRef),
      clientRef(clientRef),
      serverConfigRef(serverConfig) {}

CommandContext::~CommandContext() {}

void CommandContext::setCommandType(const std::string& cmdType) {
  this->commandType = cmdType;
}

void CommandContext::setArgs(const std::vector<std::string>& argsVec) {
  this->argsVec = argsVec;
}

const std::string& CommandContext::getCommandType() const {
  return commandType;
}

const std::vector<std::string>& CommandContext::args() const { return argsVec; }

// Use for disconnecting or replying to user
SessionID CommandContext::sessionID() const { return sessionRef.getID(); }

ClientID CommandContext::clientID() const { return clientRef.getID(); }

const IServerConfig& CommandContext::serverConfig() const {
  return serverConfigRef;
}
