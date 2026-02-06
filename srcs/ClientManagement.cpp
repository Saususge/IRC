#include "ClientManagement.hpp"

#include "ClientRegistry.hpp"

// ClientRegistry Singleton
namespace {
ClientRegistry _clientReg;
};

namespace ClientManagement {
ClientID createClient() { return _clientReg.createClient(); }
void deleteClient(ClientID id) { _clientReg.deleteClient(id); }

IClient* getClient(ClientID id) {
  (void)id;
  return _clientReg.getClient(id);
}

IClient* getClient(SessionID sessionID) {
  (void)sessionID;
  return NULL;
}

IClient* getClient(ISession* session) {
  (void)session;
  return NULL;
}

IClient* getClient(const std::string& nick) {
  (void)nick;
  return NULL;
}

const std::set<const IClient*> getClients() { return _clientReg.getClients(); }

ClientID getClientID(IClient* client) {
  (void)client;
  return ClientID(-1);
}

ClientID getClientID(const std::string& nick) {
  (void)nick;
  return ClientID(-1);
}

std::set<ClientID> getClientIDs() { return std::set<ClientID>(); }

bool isNickinUse(const std::string& nick) {
  return _clientReg.isNickinUse(nick);
}
}  // namespace ClientManagement