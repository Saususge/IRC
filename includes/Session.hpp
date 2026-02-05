#ifndef SESSION_HPP
#define SESSION_HPP

#include <map>
#include <set>
#include <string>

#include "ISession.hpp"

class SessionRegistry {
 public:
  ~SessionRegistry() { deleteScheduledSession(); }

  void addSession(ISession* session);
  ISession* getSession(int socketFD);
  void scheduleForDeletion(int socketFD);
  const std::set<int> deleteScheduledSession();

 private:
  std::set<int> _deletionQueue;
  std::map<int, ISession*> _sessions;
};

namespace SessionManagement {
void addSession(ISession* session);
ISession* getSession(int socketFD);
void scheduleForDeletion(int socketFD);
const std::set<int> deleteScheduledSession();
};  // namespace SessionManagement

class Session : public ISession {
 public:
  // Session owns a socket file descriptor
  Session(int socketFD);
  ~Session();

  // Return empty string on failure.
  std::string read();

  // Return 0 on sucess
  int send(const std::string& msg);

  int getSocketFD() const;

  static const int BUFFER_SIZE = 512;

 private:
  const int _socketFD;

  std::string _inBuf;
  std::string _outBuf;
};
#endif