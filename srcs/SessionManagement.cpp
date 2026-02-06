#include "SessionManagement.hpp"

#include "Session.hpp"

namespace {
SessionRegistry sessionReg;
};

namespace SessionManagement {
void addSession(ISession* session) { return sessionReg.addSession(session); }
ISession* getSession(int socketFD) { return sessionReg.getSession(socketFD); }

ISession* getSession(SessionID sessionID) {
  (void)sessionID;
  return sessionReg.getSession(sessionID);
}

ISession* getSession(ClientID clientID) {
  (void)clientID;
  return NULL;
}

ISession* getSession(IClient* client) {
  return getSession(client->getSessionID());
}

ISession* getSession(const std::string& nick) {
  (void)nick;
  return NULL;
}

std::set<ISession*> getSessions() { return std::set<ISession*>(); }

SessionID getSessionID(ISession* session) { return session->getID(); }

SessionID getSessionID(IClient* client) { return client->getSessionID(); }

SessionID getSessionID(const std::string& nick) {
  (void)nick;
  return SessionID(-1);
}

std::set<SessionID> getSessionIDs() {
  const std::map<int, ISession*> sessions = sessionReg.getSessions();
  std::set<SessionID> idSet;

  for (std::map<int, ISession*>::const_iterator iter = sessions.begin();
       iter != sessions.end(); iter++) {
    idSet.insert(iter->second->getID());
  }
  return idSet;
}

void scheduleForDeletion(int socketFD) {
  sessionReg.scheduleForDeletion(socketFD);
}
const std::set<int> deleteScheduledSession() {
  return sessionReg.deleteScheduledSession();
}
};  // namespace SessionManagement