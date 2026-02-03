#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

#include "IClient.hpp"
#include "ISession.hpp"

class Client : public IClient {
 public:
  Client(Session& session, IServerConfig& serverConfig);
  ~Client();
  IRC::Numeric Authenticate(const IServerConfig& serverConfig,
                            const std::string& password);
  // NICK
  IRC::Numeric setNick(IClientRegistry& registry, const std::string& nick);
  // USER
  IRC::Numeric setUserInfo(const std::string& user,
                           const std::string& realName);
  const std::string& getNick();
  const std::string& getUser();
  const std::string& getRealName();

  // ClientResistry or equivalent has to send.
  int send(const std::string& msg);

  IRC::Numeric joinChannel(IChannelRegistry& registry,
                           const std::string& channelName,
                           const std::string& key = "");
  IRC::Numeric partChannel(IChannelRegistry& registry,
                           const std::string& channelName);
  const std::vector<std::string>& getJoinedChannels();

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;

  short loginFlags;
  bool registered;

  std::vector<std::string> _joinedChannels;

  ISession& session;

  IRC::Numeric checkLoginFlags();
  std::vector<std::string>::iterator findFromJoinedChannel(
      const std::string& channelName);
};

#endif
