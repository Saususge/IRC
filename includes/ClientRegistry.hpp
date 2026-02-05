#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>
#include <string>

#include "IClientRegistry.hpp"

class ClientRegistry : public IClientRegistry {
 public:
  ClientRegistry();
  virtual ~ClientRegistry();

  bool isNickInUse(const std::string& nick) const;
  bool hasClient(const std::string& nick) const;
  const std::vector<std::string>& getClients() const;
  int send(const std::string& nick, const std::string& msg);

  // Simple management for now
  void addClient(const std::string& nick, int fd);
  void removeClient(const std::string& nick);
  void removeClientByFd(int fd);

 private:
  std::map<std::string, int> _clients;
  mutable std::vector<std::string> _cachedNickList;
};

#endif
