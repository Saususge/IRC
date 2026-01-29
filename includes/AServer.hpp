#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <map>
#include <vector>
#include <sys/poll.h>
#include <string>

#include "ISession.hpp"
#include "IServerConfig.hpp"

// Forward Declaration
class ISession;

class AServer {
 public:
  virtual ~AServer();

  // 메인 루프 실행
  void run();

  // 설정 정보 접근 (구현은 자식에서)
  virtual const IServerConfig& serverConfig() const = 0;

 protected:
  // 자식 클래스(Server)가 구현해야 할 이벤트 핸들러
  virtual void onClientConnected(int fd) = 0;
  virtual void onClientDisconnected(int fd) = 0;
  virtual void onClientMessage(int fd, const std::string& message) = 0;

  // 소켓 초기화 함수
  void initSocket(int port);

  // 세션 찾기 (자식 클래스에서 접근 가능하도록 protected)
  ISession* findSession(int fd);

 protected:
  int _listeningSocketFD;
  std::vector<struct pollfd> _pollfds;
  std::map<int, ISession*> _sessions;

 private:
  void acceptClient();
  void handlePollIn(size_t index);
  void handlePollOut(size_t index);
  void processDeletionQueue(); // 지연된 삭제 처리
};

#endif