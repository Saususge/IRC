#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <vector>

#include "ISession.hpp"

namespace SessionManager {
extern std::vector<int> deletionQueue;
void scheduleForDeletion(int socketFD);
}  // namespace SessionManager

class Session : public ISession {
 public:
  // Session owns a socket file descriptor
  Session(int socketFD);
  ~Session();

  std::string read();

  int send(const std::string& msg);

  int disconnect();

  int getSocketFD() const;

 private:
  const int _socketFD;

  std::string _inBuf;
  std::string _outBuf;
};
#endif