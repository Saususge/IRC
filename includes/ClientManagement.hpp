#ifndef CLIENTMANAGEMENT_HPP
#define CLIENTMANAGEMENT_HPP

#include "IClientRegistry.hpp"

namespace ClientManagement {
ClientID createClient();
void deleteClient(ClientID id);

IClient* getClient(ClientID id);
std::set<const IClient*> getClients();

bool isNickinUse(const std::string& nick);
}  // namespace ClientManagement

#endif