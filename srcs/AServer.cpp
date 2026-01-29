#include "AServer.hpp"
#include "Session.hpp" // SessionManager 접근 및 Session 생성용
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

AServer::~AServer() {
    // 모든 세션 정리
    for (std::map<int, ISession*>::iterator it = _sessions.begin(); it != _sessions.end(); ++it) {
        delete it->second;
    }
    _sessions.clear();
    if (_listeningSocketFD != -1) ::close(_listeningSocketFD);
}

void AServer::initSocket(int port) {
    _listeningSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (_listeningSocketFD < 0) throw std::runtime_error("Socket creation failed");

    int opt = 1;
    setsockopt(_listeningSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_listeningSocketFD, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_listeningSocketFD, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Bind failed");

    if (listen(_listeningSocketFD, SOMAXCONN) < 0)
        throw std::runtime_error("Listen failed");

    struct pollfd pfd;
    pfd.fd = _listeningSocketFD;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    std::cout << "Server listening on port " << port << std::endl;
}

void AServer::run() {
    while (true) {
        if (poll(&_pollfds[0], _pollfds.size(), -1) < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        for (size_t i = 0; i < _pollfds.size(); ++i) {
            if (_pollfds[i].revents == 0) continue;

            if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                SessionManager::scheduleForDeletion(_pollfds[i].fd);
                continue;
            }

            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _listeningSocketFD)
                    acceptClient();
                else
                    handlePollIn(i);
            }

            // POLLOUT 로직은 Session 내부 버퍼링 정책에 따라 필요 시 구현
            // if (i < _pollfds.size() && (_pollfds[i].revents & POLLOUT)) handlePollOut(i);
        }

        processDeletionQueue();
    }
}

void AServer::acceptClient() {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(_listeningSocketFD, (struct sockaddr*)&clientAddr, &len);

    if (clientFd < 0) return;

    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN | POLLOUT; // POLLOUT 감시 필요 시 추가
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    _sessions[clientFd] = new Session(clientFd);

    onClientConnected(clientFd);
}

void AServer::handlePollIn(size_t index) {
    int fd = _pollfds[index].fd;
    if (_sessions.find(fd) == _sessions.end()) return;

    // Session::read() 내부에서 EOF 감지 시 scheduleForDeletion 호출됨
    std::string data = _sessions[fd]->read();
    
    if (!data.empty()) {
        onClientMessage(fd, data);
    }
}

void AServer::handlePollOut(size_t index) {
    (void)index;
    // 필요한 경우 구현
}

void AServer::processDeletionQueue() {
    if (SessionManager::deletionQueue.empty()) return;

    for (size_t i = 0; i < SessionManager::deletionQueue.size(); ++i) {
        int fd = SessionManager::deletionQueue[i];

        onClientDisconnected(fd); // 비즈니스 로직 정리

        if (_sessions.find(fd) != _sessions.end()) {
            delete _sessions[fd]; // 소켓 close는 여기서 발생 (Session 소멸자)
            _sessions.erase(fd);
        }

        for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it) {
            if (it->fd == fd) {
                _pollfds.erase(it);
                break;
            }
        }
    }
    SessionManager::deletionQueue.clear();
}

ISession* AServer::findSession(int fd) {
    if (_sessions.find(fd) != _sessions.end())
        return _sessions[fd];
    return NULL;
}