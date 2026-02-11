#include "Command.hpp"

CommandContext::CommandContext(ISession& sessionRef, IClient& clientRef,
                               IClientRegistry& clientRegistry,
                               IChannelRegistry& channelRegistry,
                               const IServerConfig& serverConfig)
    : sessionRef(sessionRef),
      clientRef(clientRef),
      clientRegistry(clientRegistry),
      channelRegistry(channelRegistry),
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
ISession& CommandContext::requester() const { return sessionRef; }

IClient& CommandContext::requesterClient() const { return clientRef; }

IClientRegistry& CommandContext::clients() const { return clientRegistry; }

IChannelRegistry& CommandContext::channels() const { return channelRegistry; }

const IServerConfig& CommandContext::serverConfig() const {
  return serverConfigRef;
}
