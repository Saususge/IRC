#ifndef CLIENTMANAGEMENT_HPP
#define CLIENTMANAGEMENT_HPP

#include <set>

#include "IClient.hpp"
#include "ISession.hpp"

namespace ClientManagement {
ClientID createClient();
void deleteClient(ClientID id);

IClient* getClient(ClientID id);
IClient* getClient(SessionID sessionID);
IClient* getClient(ISession* session);
IClient* getClient(const std::string& nick);
const std::set<const IClient*> getClients();

ClientID getClientID(IClient* client);
ClientID getClientID(const std::string& nick);
std::set<ClientID> getClientIDs();
}  // namespace ClientManagement

#endif