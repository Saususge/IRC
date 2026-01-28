#ifndef ICLIENTREGISTRY_HPP
#define ICLIENTREGISTRY_HPP

#include <string>
#include <vector>

#include "IClient.hpp"

class IClientRegistry {
 public:
  virtual ~IClientRegistry() {};

  // May need shared ptr for parameter
  virtual int registerClient(IClient* client);
  virtual bool isNickInUse(const std::string& nick) const = 0;
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual std::vector<const std::string&> getAllClients() const = 0;

  virtual int send(const std::string& nick, const std::string& msg) = 0;
};
#endif  // ICLIENTREGISTRY_HPP