#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <string>
#include <vector>

#include "IChannelRegistry.hpp"
#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"
#include "numeric.hpp"

class ICommandContext {
 public:
  virtual ~ICommandContext() {};

  virtual const std::string& getCommandType() const = 0;
  virtual const std::vector<const std::string> args() const = 0;
  // Use for disconnecting or replying to user
  virtual ISession& requester() = 0;

  virtual IClientRegistry& clients() = 0;
  virtual IChannelRegistry& channels() = 0;
  virtual const IServerConfig& serverConfig() const = 0;
};

class ICommand {
 public:
  virtual ~ICommand();

  virtual IRC::Numeric excute(ICommandContext& ctx) const;
};

#endif