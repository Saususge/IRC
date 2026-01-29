#include "AServer.hpp"
#include "Session.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <algorithm>
#include <cerrno>

AServer::~AServer() {
    for (std::map<int, ISession*>::iterator it = _sessions.begin(); it != _sessions.end(); ++it) { delete it->second;}
    _sessions.clear();
    if (_listeningSocketFD != -1) ::close(_listeningSocketFD);
}

void AServer::initSocket(int port) {
    _listeningSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (_listeningSocketFD < 0) throw std::runtime_error("Socket creation failed");

    int opt = 1;
    if (setsockopt(_listeningSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Setsockopt failed");

    if (fcntl(_listeningSocketFD, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Fcntl failed");

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

    std::cout << "Server listening on port " << port << "..." << std::endl;
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

            // R/W Events
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _listeningSocketFD)
                    acceptClient();
                else
                    handlePollIn(i);
            }
            if (_pollfds[i].revents & POLLOUT) {
                handlePollOut(i);
            }
        }
        processDeletionQueue();
    }
}

void AServer::acceptClient() {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(_listeningSocketFD, (struct sockaddr*)&clientAddr, &len);

    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) perror("accept");
        return;
    }

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl");
        ::close(clientFd);
        return;
    }

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN | POLLOUT;
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    _sessions[clientFd] = new Session(clientFd);

    onClientConnected(clientFd);
}

void AServer::handlePollIn(size_t index) {
    int fd = _pollfds[index].fd;
    ISession* session = findSession(fd);
    if (!session) return;

    std::string data = session->read();
    
    if (!data.empty()) {
        onClientMessage(fd, data);
    }
}

void AServer::handlePollOut(size_t index) {
    int fd = _pollfds[index].fd;
    ISession* session = findSession(fd);
    if (!session) return;

    session->send(""); 
}

void AServer::processDeletionQueue() {
    if (SessionManager::deletionQueue.empty()) return;

    for (size_t i = 0; i < SessionManager::deletionQueue.size(); ++i) {
        int fd = SessionManager::deletionQueue[i];

        onClientDisconnected(fd);

        if (_sessions.find(fd) != _sessions.end()) {
            delete _sessions[fd];
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
    if (_sessions.find(fd) != _sessions.end()) return _sessions[fd];
    return NULL;
}