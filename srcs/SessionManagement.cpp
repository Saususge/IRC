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
  return NULL;
}

ISession* getSession(ClientID clientID) {
  (void)clientID;
  return NULL;
}

ISession* getSession(IClient* client) {
  (void)client;
  return NULL;
}

ISession* getSession(const std::string& nick) {
  (void)nick;
  return NULL;
}

std::set<ISession*> getSessions() { return std::set<ISession*>(); }

SessionID getSessionID(ISession* session) {
  (void)session;
  return SessionID(-1);
}

SessionID getSessionID(IClient* client) {
  (void)client;
  return SessionID(-1);
}

SessionID getSessionID(const std::string& nick) {
  (void)nick;
  return SessionID(-1);
}

std::set<SessionID> getSessionIDs() { return std::set<SessionID>(); }

void scheduleForDeletion(int socketFD) {
  sessionReg.scheduleForDeletion(socketFD);
}
const std::set<int> deleteScheduledSession() {
  return sessionReg.deleteScheduledSession();
}
};  // namespace SessionManagement