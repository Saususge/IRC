#include <map>
#include <set>

#include "ISession.hpp"
#include "Session.hpp"

void SessionRegistry::addSession(ISession* session) {
  _sessions[session->getSocketFD()] = session;
}

void SessionRegistry::scheduleForDeletion(int socketFD,
                                          ISession::SessionStatus status) {
  ISession* session = getSession(socketFD);
  if (session == NULL) return;
  session->setStatus(status);
  _deletionQueue.insert(socketFD);
}

ISession* SessionRegistry::getSession(int socketFD) {
  std::map<int, ISession*>::iterator sessionIter = _sessions.find(socketFD);
  if (sessionIter == _sessions.end()) {
    return NULL;
  }
  return sessionIter->second;
}

ISession* SessionRegistry::getSession(SessionID id) {
  std::map<int, ISession*>::iterator sessionIter = _sessions.begin();
  for (; sessionIter != _sessions.end(); sessionIter++) {
    if ((sessionIter->second)->getID() == id) return sessionIter->second;
  }
  return NULL;
}

const std::map<int, ISession*>& SessionRegistry::getSessions() {
  return _sessions;
}

const std::set<int> SessionRegistry::deleteScheduledSession() {
  for (std::set<int>::iterator it = _deletionQueue.begin();
       it != _deletionQueue.end(); ++it) {
    int fd = *it;
    std::map<int, ISession*>::iterator sessionIter = _sessions.find(fd);
    if (sessionIter != _sessions.end()) {
      ISession::SessionStatus status = sessionIter->second->getStatus();

      if (status == ISession::DEAD || (status == ISession::CLOSING &&
                                       sessionIter->second->isOutBufEmpty())) {
        delete _sessions[fd];
        _sessions.erase(fd);
        _deletionQueue.erase(fd);
      }
    }
  }
  const std::set<int> ret = _deletionQueue;
  return ret;
}
