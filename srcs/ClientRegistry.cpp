#include "ClientRegistry.hpp"

#include <map>

#include "Client.hpp"
#include "IClient.hpp"
#include "IClientRegistry.hpp"

ClientRegistry::ClientRegistry() : _nextClientID(0) {}
ClientRegistry::~ClientRegistry() {
  for (iterator it = _clients.begin(); it != _clients.end(); ++it) {
    delete it->second;
  }
};

ClientID ClientRegistry::createClient() {
  _clients[_nextClientID] = new Client();
  return _nextClientID++;
}

void ClientRegistry::deleteClient(ClientID id) {
  iterator it = _clients.find(id);
  if (it == _clients.end()) {
    return;
  }
  delete it->second;
  _clients.erase(it);
}

IClient* ClientRegistry::getClient(ClientID id) {
  iterator it = _clients.find(id);
  if (it == _clients.end()) {
    return NULL;
  }
  return it->second;
}

const std::set<IClient*> ClientRegistry::getClients() {
  std::set<IClient*> ret;
  for (iterator it = _clients.begin(); it != _clients.end(); ++it) {
    ret.insert(it->second);
  }
  return ret;
}

bool ClientRegistry::isNickinUse(const std::string& nick) {
  for (iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second->getNick() == nick) {
      return true;
    }
  }
  return false;
}
