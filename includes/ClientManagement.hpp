#ifndef CLIENTMANAGEMENT_HPP
#define CLIENTMANAGEMENT_HPP

#include <set>

#include "IClient.hpp"
#include "IClientRegistry.hpp"

namespace ClientManagement {
ClientID createClient();
void deleteClient(ClientID id);

IClient* getClient(ClientID id);
IClient* getClient(SessionID sessionID);
IClient* getClient(const std::string& nick);
std::set<const IClient*> getClients();

ClientID getClientID(IClient* client);
std::set<ClientID> getClientIDs();
}  // namespace ClientManagement

#endif