#ifndef ICLIENTREGISTRY_HPP
#define ICLIENTREGISTRY_HPP

#include <string>
#include <vector>

class IClientRegistry {
 public:
  virtual ~IClientRegistry() {};

  // May need shared ptr for parameter
  // registerClient will be done by ISession or equivalent
  // virtual int registerClient(IClient* client);
  virtual bool isNickInUse(const std::string& nick) const = 0;
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() const = 0;

  virtual int send(const std::string& nick, const std::string& msg) = 0;
};
#endif  // ICLIENTREGISTRY_HPP