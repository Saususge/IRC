#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <string>
#include <vector>

#include "IChannelRegistry.hpp"
#include "IClient.hpp"
#include "IClientRegistry.hpp"
#include "IServerConfig.hpp"
#include "ISession.hpp"
#include "numeric.hpp"

class ICommandContext {
 public:
  virtual ~ICommandContext() {};

  virtual const std::string& getCommandType() const = 0;
  virtual const std::vector<std::string>& args() const = 0;
  // Use for disconnecting or replying to user
  virtual ISession& requester() const = 0;
  virtual IClient& requesterClient() const = 0;

  virtual IClientRegistry& clients() const = 0;
  virtual IChannelRegistry& channels() const = 0;
  virtual const IServerConfig& serverConfig() const = 0;
};

class ICommand {
 public:
  virtual ~ICommand() {}

  virtual IRC::Numeric execute(ICommandContext& ctx) const = 0;
};

#endif