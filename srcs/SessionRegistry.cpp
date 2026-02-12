#include <map>
#include <set>

#include "ChannelManagement.hpp"
#include "ClientManagement.hpp"
#include "CommandUtility.hpp"
#include "ISession.hpp"
#include "Session.hpp"

void SessionRegistry::addSession(ISession* session) {
  _sessions[session->getSocketFD()] = session;
  session->setID(_nextSessionID++);
}

void SessionRegistry::scheduleForDeletion(int socketFD,
                                          ISession::SessionStatus status) {
  ISession* session = getSession(socketFD);
  if (session == NULL) return;
  // prevent DEAD -> CLOSING overwrite
  if (session->getStatus() != ISession::DEAD) session->setStatus(status);
  _deletionQueue.insert(socketFD);
}

void SessionRegistry::scheduleForDeletion(SessionID sessionID,
                                          ISession::SessionStatus status) {
  ISession* session = getSession(sessionID);
  if (session == NULL) return;
  // prevent DEAD -> CLOSING overwrite
  if (session->getStatus() != ISession::DEAD) session->setStatus(status);
  _deletionQueue.insert(session->getSocketFD());
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
  std::set<int> ret;

  for (std::set<int>::iterator it = _deletionQueue.begin();
       it != _deletionQueue.end(); ++it) {
    int fd = *it;
    std::map<int, ISession*>::iterator sessionIter = _sessions.find(fd);
    if (sessionIter != _sessions.end()) {
      ISession::SessionStatus status = sessionIter->second->getStatus();

      if (status == ISession::DEAD || (status == ISession::CLOSING &&
                                       sessionIter->second->isOutBufEmpty())) {
        ClientID clientID = sessionIter->second->getClientID();
        IClient* client = ClientManagement::getClient(clientID);

        if (client != NULL) ClientManagement::deleteClient(clientID);
        delete sessionIter->second;
        _sessions.erase(sessionIter);
        ret.insert(fd);
      }
    } else {
      // erase orphan file descriptor
      ret.insert(fd);
    }
  }

  for (std::set<int>::iterator i = ret.begin(); i != ret.end(); i++)
    _deletionQueue.erase(*i);
  return ret;
}
