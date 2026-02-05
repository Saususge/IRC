#ifndef CLIENTMANAGEMENT_HPP
#define CLIENTMANAGEMENT_HPP

#include <set>

#include "IClientRegistry.hpp"

namespace ClientManagement {
ClientID createClient();
void deleteClient(ClientID id);

IClient* getClient(ClientID id);
std::set<const IClient*> getClients();
std::set<ClientID> getClientIDs();

bool isNickinUse(const std::string& nick);
}  // namespace ClientManagement

#endif