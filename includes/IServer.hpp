#ifndef ISERVER_HPP
#define ISERVER_HPP

#include <string>
#include <vector>

#include "IChannel.hpp"
#include "IClient.hpp"
#include "ISession.hpp"

class IServer {
 public:
  virtual ~IServer() {};

  virtual void registerClient(IClient* client) = 0;
  virtual bool isNickInUse(const std::string& nick) = 0;
  virtual const IClient* findClient(const std::string& nick) = 0;
  virtual std::vector<const IClient*> getAllClients() = 0;

  virtual int createChannel(const std::string& channelName) = 0;
  virtual int deleteChannel(const std::string& channelName) = 0;
  virtual IChannel* findChannel(const std::string& channelName) = 0;
  virtual std::vector<const IChannel*> getChannels() = 0;

  virtual int acceptSession() = 0;
  // Defererd disconnection
  virtual int disconnectSession(ISession* session) = 0;

  virtual int run() = 0;
};
#endif