#include <cassert>

#include "Command.hpp"

namespace {

class CommandPool {
 public:
  CommandPool() {
    _pool["PASS"] = &_passCmd;
    _pool["NICK"] = &_nickCmd;
    _pool["USER"] = &_userCmd;
    _pool["QUIT"] = &_quitCmd;
    _pool["JOIN"] = &_joinCmd;
    _pool["PART"] = &_partCmd;
    _pool["MODE"] = &_modeCmd;
    _pool["TOPIC"] = &_topicCmd;
    _pool["NAMES"] = &_namesCmd;
    _pool["INVITE"] = &_inviteCmd;
    _pool["KICK"] = &_kickCmd;
    _pool["PRIVMSG"] = &_privmsgCmd;
    _pool["NOTICE"] = &_noticeCmd;
    _pool["DCC"] = &_dccSendCmd;
  };
  const std::map<std::string, ICommand*>& getPool() { return _pool; }

 private:
  std::map<std::string, ICommand*> _pool;
  PassCommand _passCmd;
  NickCommand _nickCmd;
  UserCommand _userCmd;
  QuitCommand _quitCmd;
  JoinCommand _joinCmd;
  PartCommand _partCmd;
  ChannelModeCommand _modeCmd;
  TopicCommand _topicCmd;
  NamesCommand _namesCmd;
  InviteCommand _inviteCmd;
  KickCommand _kickCmd;
  PrivmsgCommand _privmsgCmd;
  NoticeCommand _noticeCmd;
  DccSendCommand _dccSendCmd;
};

CommandPool _pool;
};  // namespace

namespace CommandFactory {

const ICommand& getCommand(const std::string& cmd) {
  if (_pool.getPool().find(cmd) == _pool.getPool().end()) {
    assert(0 && "Unexpected Command");
  }
  return *_pool.getPool().find(cmd)->second;
}
}  // namespace CommandFactory