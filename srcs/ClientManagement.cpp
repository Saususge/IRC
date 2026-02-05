#include "ClientRegistry.hpp"

// ClientRegistry Singleton
namespace {
ClientRegistry _clientReg;
};

namespace ClientManagement {
ClientID createClient() { return _clientReg.createClient(); }
void deleteClient(ClientID id) { _clientReg.deleteClient(id); }

IClient* getClient(ClientID id) { return _clientReg.getClient(id); }
const std::set<IClient*> getClients() { return _clientReg.getClients(); }
}  // namespace ClientManagement