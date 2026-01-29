// [TEMPORARY] Mock implementation by AI for Server testing. 
// TODO: Replace with implementation from the Client/Session team later.
#include "ClientRegistry.hpp"
#include "ClientRegistry.hpp"

ClientRegistry::ClientRegistry() {}
ClientRegistry::~ClientRegistry() {}

bool ClientRegistry::isNickInUse(const std::string& nick) const {
    return _clients.find(nick) != _clients.end();
}

bool ClientRegistry::hasClient(const std::string& nick) const {
    return isNickInUse(nick);
}

const std::vector<const std::string>& ClientRegistry::getClients() const {
    return _cachedNickList;
}

int ClientRegistry::send(const std::string& nick, const std::string& msg) {
    (void)nick;
    (void)msg;
    return 0;
}

void ClientRegistry::addClient(const std::string& nick) {
    _clients[nick] = 1;
}

void ClientRegistry::removeClient(const std::string& nick) {
    _clients.erase(nick);
}
