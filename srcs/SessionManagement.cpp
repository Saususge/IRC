#include "Session.hpp"

namespace {
SessionRegistry sessionReg;
};

namespace SessionManagement {
void addSession(ISession* session) { return sessionReg.addSession(session); }
ISession* getSession(int socketFD) { return sessionReg.getSession(socketFD); }

void scheduleForDeletion(int socketFD) {
  sessionReg.scheduleForDeletion(socketFD);
}
const std::set<int> deleteScheduledSession() {
  return sessionReg.deleteScheduledSession();
}
};  // namespace SessionManagement