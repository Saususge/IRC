#ifndef CLIENTMANAGEMENT_HPP
#define CLIENTMANAGEMENT_HPP

#include "IClientRegistry.hpp"

namespace ClientManagement {
ClientID createClient();
void deleteClient(ClientID id);

IClient* getClient(ClientID id);
const std::set<IClient*> getClients();
}  // namespace ClientManagement

#endif