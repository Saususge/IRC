#ifndef ICHANNEL_HPP
#define ICHANNEL_HPP

#include <string>
#include <vector>

class IChannel {
 public:
  // Bit mask
  typedef int IChannelMode;

  virtual ~IChannel() {};

  virtual const std::string& getChannelName() const = 0;

  // Use `except` only for exclusing the sender
  virtual int broadcast(const std::string& msg,
                        const std::string& except = "") = 0;

  virtual int addClient(const std::string& nick) = 0;
  virtual void removeClient(const std::string& nick) = 0;
  virtual bool hasClient(const std::string& nick) const = 0;
  virtual int setClientOp(const std::string& nick) = 0;
  virtual int unsetClientOp(const std::string& nick) = 0;
  virtual int isClientOp(const std::string& nick) const = 0;
  virtual const std::vector<const std::string>& getClients() = 0;

  virtual int getClientNumber() const = 0;

  virtual int setMode(const std::string& reqeusterNcik, IChannelMode mode) = 0;
  virtual int addMode(const std::string& reqeusterNcik, IChannelMode mode) = 0;
  virtual int removeMode(const std::string& reqeusterNcik,
                         IChannelMode mode) = 0;
  virtual IChannelMode getMode() const = 0;

  virtual int addToInviteList(const std::string& requesterNick,
                              const std::string& targetNick) = 0;
  virtual int removeFromInviteList(const std::string& requesterNick,
                                   const std::string& targetNick) = 0;
  virtual bool isInInviteList(const std::string& nick) const = 0;
};
#endif