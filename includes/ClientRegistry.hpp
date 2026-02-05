#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include "IClientRegistry.hpp"
#include <map>
#include <string>

class ClientRegistry : public IClientRegistry {
 public:
  ClientRegistry();
  virtual ~ClientRegistry();

  bool isNickInUse(const std::string& nick) const;
  bool hasClient(const std::string& nick) const;
  const std::vector<const std::string>& getClients() const;
  int send(const std::string& nick, const std::string& msg);

  // Simple management for now
  void addClient(const std::string& nick, int fd);
  void removeClient(const std::string& nick);
  void removeClientByFd(int fd);

 private:
  std::map<std::string, int> _clients;
  mutable std::vector<const std::string> _cachedNickList;
};

#endif
