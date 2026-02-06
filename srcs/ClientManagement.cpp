#include "ClientManagement.hpp"

#include "ClientRegistry.hpp"

// ClientRegistry Singleton
namespace {
ClientRegistry _clientReg;
};

namespace ClientManagement {
ClientID createClient() { return _clientReg.createClient(); }
void deleteClient(ClientID id) { _clientReg.deleteClient(id); }

IClient* getClient(ClientID id) { return _clientReg.getClient(id); }

IClient* getClient(SessionID sessionID) {
  (void)sessionID;
  return NULL;
}

IClient* getClient(ISession* session) {
  return getClient(session->getClientID());
}

IClient* getClient(const std::string& nick) {
  return getClient(getClientID(nick));
}

const std::set<const IClient*> getClients() { return _clientReg.getClients(); }

ClientID getClientID(IClient* client) { return client->getID(); }

ClientID getClientID(const std::string& nick) {
  const std::set<const IClient*> clients = _clientReg.getClients();
  for (std::set<const IClient*>::const_iterator iter = clients.begin();
       iter != clients.end(); iter++) {
    if ((*iter)->getNick() == nick) return (*iter)->getID();
  }
  return ClientID(-1);
}

std::set<ClientID> getClientIDs() {
  const std::set<const IClient*> clients = _clientReg.getClients();
  std::set<ClientID> idSet;
  for (std::set<const IClient*>::const_iterator iter = clients.begin();
       iter != clients.end(); iter++) {
    idSet.insert((*iter)->getID());
  }
  return idSet;
}

bool isNickinUse(const std::string& nick) {
  return _clientReg.isNickinUse(nick);
}
}  // namespace ClientManagement