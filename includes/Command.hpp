#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <map>
#include <string>

#include "ICommand.hpp"
#include "numeric.hpp"

class CommandContext : public ICommandContext {
 public:
  CommandContext(ISession& sessionRef, IClient& clientRef,
                 IClientRegistry& clientRegistry,
                 IChannelRegistry& channelRegistry,
                 const IServerConfig& serverConfig);
  ~CommandContext();

  void setCommandType(const std::string& cmdType);
  void setArgs(const std::vector<std::string>& argsVec);
  const std::string& getCommandType() const;
  const std::vector<std::string>& args() const;
  // Use for disconnecting or replying to user
  ISession& requester() const;
  IClient& requesterClient() const;

  IClientRegistry& clients() const;
  IChannelRegistry& channels() const;
  const IServerConfig& serverConfig() const;

 private:
  std::string commandType;
  std::vector<std::string> argsVec;

  ISession& sessionRef;
  IClient& clientRef;
  IClientRegistry& clientRegistry;
  IChannelRegistry& channelRegistry;
  const IServerConfig& serverConfigRef;
};

// 3.1.1 Password message
class PassCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.1.2 Nick message
class NickCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.1.3 User message
class UserCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// Omit Mode, Service, Squit message

// 3.1.7 Quit
class QuitCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.1 Join message
class JoinCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.2 Part message
class PartCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.3 Channel mode message
class ChannelModeCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.4 Topic message
class TopicCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.5 Names message
class NamesCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.6 List message
// Omitted

// 3.2.7 Invite message
class InviteCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.2.8 Kick command
class KickCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.3.1 Private messages
class PrivmsgCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// 3.3.2 Notice
class NoticeCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const;
};

// DCC SEND (CTCP)
class DccSendCommand : public ICommand {
  IRC::Numeric execute(ICommandContext& ctx) const {
    (void)ctx;
    return IRC::DO_NOTHING;
  }
};

namespace CommandFactory {
const ICommand& getCommand(const std::string& cmd);
};

#endif