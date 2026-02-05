#include <map>
#include <set>

#include "ISession.hpp"
#include "Session.hpp"

namespace SessionManagement {
SessionRegistry sessionReg;

void addSession(ISession* session) { return sessionReg.addSession(session); }
ISession* getSession(int socketFD) { return sessionReg.getSession(socketFD); }

void scheduleForDeletion(int socketFD) {
  sessionReg.scheduleForDeletion(socketFD);
}
const std::set<int> deleteScheduledSession() {
  return sessionReg.deleteScheduledSession();
}
};  // namespace SessionManagement

void SessionRegistry::addSession(ISession* session) {
  _sessions[session->getSocketFD()] = session;
}

void SessionRegistry::scheduleForDeletion(int socketFD) {
  _deletionQueue.insert(socketFD);
}

ISession* SessionRegistry::getSession(int socketFD) {
  std::map<int, ISession*>::iterator sessionIter = _sessions.find(socketFD);
  if (sessionIter == _sessions.end()) {
    return NULL;
  }
  return sessionIter->second;
}

const std::set<int> SessionRegistry::deleteScheduledSession() {
  for (std::set<int>::iterator it = _deletionQueue.begin();
       it != _deletionQueue.end(); ++it) {
    int fd = *it;
    if (_sessions.find(fd) != _sessions.end()) {
      delete _sessions[fd];
      _sessions.erase(fd);
    }
  }
  const std::set<int> ret = _deletionQueue;
  _deletionQueue.clear();
  return ret;
}
