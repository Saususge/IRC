#ifndef CLIENTRESISTRY_HPP
#define CLIENTRESISTRY_HPP

#include "IClientRegistry.hpp"

class ClientRegistry : public IClientRegistry {
  // May need shared ptr for parameter
  // registerClient will be done by ISession or equivalent
  // virtual int registerClient(IClient* client);
  virtual bool isNickInUse(const std::string& nick) const = 0;
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() const = 0;

  virtual bool isRegistered(const std::string& nick) = 0;
  // Register implicitly when session is connected
  // virtual int Register(const std::string& nick) = 0;
  virtual int removeClient(const std::string& nick) = 0;
  // removeClient(ISession)

  virtual int send(const std::string& nick, const std::string& msg) = 0;
};

#endif
