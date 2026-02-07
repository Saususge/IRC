#include "Session.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "SessionManagement.hpp"

Session::Session(int socketFD)
    : _socketFD(socketFD), _sessionID(-1), _clientID(-1) {}
Session::~Session() { close(_socketFD); }

std::string Session::read() {
  char buf[BUFFER_SIZE];

  ssize_t n = recv(_socketFD, buf, sizeof(buf), 0);
  if (n <= 0) {
    SessionManagement::scheduleForDeletion(_socketFD);
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

void Session::enqueueMsg(const std::string& msg) { _outBuf.append(msg); }

int Session::send() {
  if (_outBuf.empty()) return 0;

  ssize_t n = ::send(_socketFD, _outBuf.c_str(), _outBuf.size(), 0);
  if (n < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      SessionManagement::scheduleForDeletion(_socketFD);
      return 1;
    }
  }
  _outBuf.erase(0, n);
  return 0;
}

int Session::getSocketFD() const { return _socketFD; }

ClientID Session::getClientID() const { return _clientID; }

SessionID Session::getID() const { return _sessionID; }