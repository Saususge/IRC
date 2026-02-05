#ifndef ICLIENTREGISTRY_HPP
#define ICLIENTREGISTRY_HPP

#include <set>

#include "IClient.hpp"

typedef std::size_t ClientID;

class IClientRegistry {
 public:
  virtual ~IClientRegistry() {};

  virtual ClientID createClient() = 0;
  virtual void deleteClient(ClientID id) = 0;

  virtual IClient* getClient(ClientID id) = 0;
  virtual const std::set<IClient*> getClients() = 0;
};
#endif  // ICLIENTREGISTRY_HPP