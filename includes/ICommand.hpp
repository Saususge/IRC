#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <string>
#include <vector>

#include "IServerConfig.hpp"
#include "defs.hpp"
#include "numeric.hpp"

class ICommandContext {
 public:
  virtual ~ICommandContext() {};

  virtual void setCommandType(const std::string& cmdType) = 0;
  virtual void setArgs(const std::vector<std::string>& argsVec) = 0;
  virtual const std::string& getCommandType() const = 0;
  virtual const std::vector<std::string>& args() const = 0;
  // Use for disconnecting or replying to user
  virtual SessionID sessionID() const = 0;
  virtual ClientID clientID() const = 0;

  // virtual IClientRegistry& clients() const = 0;
  // virtual IChannelRegistry& channels() const = 0;
  // virtual const IServerConfig& serverConfig() const = 0;
};

class ICommand {
 public:
  virtual ~ICommand() {}

  virtual IRC::Numeric execute(ICommandContext& ctx) const = 0;
};

#endif