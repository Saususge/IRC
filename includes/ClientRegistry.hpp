#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>

#include "IClientRegistry.hpp"

class ClientRegistry : public IClientRegistry {
 public:
  ClientRegistry();
  ~ClientRegistry();

  ClientID createClient();
  void deleteClient(ClientID id);

  IClient* getClient(ClientID id);
  const std::set<IClient*> getClients();

 private:
  std::map<ClientID, IClient*> _clients;
  ClientID _nextClientID;
  typedef std::map<ClientID, IClient*>::iterator iterator;
};

#endif
