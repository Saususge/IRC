#ifndef SESSION_HPP
#define SESSION_HPP

#include <map>
#include <set>
#include <string>

#include "IClient.hpp"
#include "IClientRegistry.hpp"
#include "ISession.hpp"
#include "defs.hpp"

class SessionRegistry {
 public:
  ~SessionRegistry() { deleteScheduledSession(); }

  void addSession(ISession* session);
  ISession* getSession(int socketFD);
  ISession* getSession(SessionID id);
  const std::map<int, ISession*>& getSessions();
  void scheduleForDeletion(int socketFD, ISession::SessionStatus status);
  const std::set<int> deleteScheduledSession();

 private:
  std::set<int> _deletionQueue;
  std::map<int, ISession*> _sessions;
};

class Session : public ISession {
 public:
  // Session owns a socket file descriptor
  Session(int socketFD, ClientID id);
  ~Session();

  // Return empty string on failure.
  int read();
  std::string readLine();

  void enqueueMsg(const std::string& msg);
  // Return 0 on sucess
  int send();

  int getSocketFD() const;

  SessionID getID() const;

  void setClientID(ClientID id);
  ClientID getClientID() const;

  void setStatus(ISession::SessionStatus status);
  ISession::SessionStatus getStatus();

  bool isOutBufEmpty();

  static const int BUFFER_SIZE = 512;

 private:
  ISession::SessionStatus _status;
  const int _socketFD;

  std::string _inBuf;
  std::string _outBuf;

  SessionID _sessionID;
  ClientID _clientID;
};
#endif