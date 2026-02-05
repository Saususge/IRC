#include "Session.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <set>

#include "ISession.hpp"

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

namespace SessionManagement {
SessionRegistry sessionReg;
};  // namespace SessionManagement

Session::Session(int socketFD) : _socketFD(socketFD) {}
Session::~Session() { close(_socketFD); }

std::string Session::read() {
  char buf[BUFFER_SIZE];

  ssize_t n = recv(_socketFD, buf, sizeof(buf), 0);
  if (n <= 0) {
    SessionManagement::sessionReg.scheduleForDeletion(_socketFD);
    return "";
  }

  std::string line;
  size_t pos;
  while ((pos = _inBuf.find("\r\n")) != std::string::npos) {
    line = _inBuf.substr(0, pos);
    _inBuf.erase(0, pos + 2);
    return line;
  }
  while ((pos = _inBuf.find("\n")) != std::string::npos) {
    line = _inBuf.substr(0, pos);
    if (!line.empty() && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);
    _inBuf.erase(0, pos + 1);
    return line;
  }
  return "";
}

int Session::send(const std::string& msg) {
  _outBuf.append(msg);

  if (_outBuf.empty()) return 0;

  ssize_t n = ::send(_socketFD, _outBuf.c_str(), _outBuf.size(), 0);
  if (n < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      SessionManagement::sessionReg.scheduleForDeletion(_socketFD);
      return 1;
    }
  }
  _outBuf.erase(0, n);
  return 0;
}

int Session::getSocketFD() const { return _socketFD; }
