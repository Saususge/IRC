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

void ClientRegistry::addClient(const std::string& nick, int fd) {
    _clients[nick] = fd;
}

void ClientRegistry::removeClient(const std::string& nick) {
    _clients.erase(nick);
}

void ClientRegistry::removeClientByFd(int fd) {
    for (std::map<std::string, int>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->second == fd) {
            _clients.erase(it);
            return;
        }
    }
}
