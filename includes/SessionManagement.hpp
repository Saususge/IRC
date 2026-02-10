#ifndef SESSIONMANAGEMENT_HPP
#define SESSIONMANAGEMENT_HPP

#include <set>

#include "IClient.hpp"
#include "ISession.hpp"

namespace SessionManagement {
void addSession(ISession* session);
void scheduleForDeletion(int socketFD, ISession::SessionStatus status);
const std::set<int> deleteScheduledSession();

ISession* getSession(int socketFD);
ISession* getSession(SessionID sessionID);
ISession* getSession(ClientID clientID);
ISession* getSession(IClient* client);
ISession* getSession(const std::string& nick);
std::set<ISession*> getSessions();

SessionID getSessionID(ISession* session);
SessionID getSessionID(IClient* client);
SessionID getSessionID(const std::string& nick);
std::set<SessionID> getSessionIDs();
};  // namespace SessionManagement
#endif