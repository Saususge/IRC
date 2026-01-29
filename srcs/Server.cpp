#include "Server.hpp"
#include "Response.hpp" // 응답 생성용
#include <iostream>
#include <sstream>

Server::Server(int port, const std::string& password) 
    : _config(password) {
    initSocket(port); // AServer 초기화
}

Server::~Server() {}

const IServerConfig& Server::serverConfig() const {
    return _config;
}

void Server::onClientConnected(int fd) {
    // 새로운 클라이언트 등록 (Manager가 하던 일)
    if (_users.find(fd) == _users.end()) {
        _users[fd] = Client(); // 기본 생성
        std::cout << "Server: Client " << fd << " connected." << std::endl;
    }
}

void Server::onClientDisconnected(int fd) {
    // 클라이언트 제거 및 채널에서 퇴장 처리 (Manager가 하던 일)
    if (_users.find(fd) != _users.end()) {
        std::string nick = _users[fd].getNickname();
        // TODO: _channels 순회하며 해당 유저 제거 로직 필요
        _users.erase(fd);
        std::cout << "Server: Client " << fd << " disconnected." << std::endl;
    }
}

void Server::onClientMessage(int fd, const std::string& message) {
    std::cout << "Received from " << fd << ": " << message << std::endl;
    dispatchCommand(fd, message);
}

void Server::dispatchCommand(int fd, const std::string& commandLine) {
    std::stringstream ss(commandLine);
    std::string command;
    ss >> command;

    // TODO: 대체된 커맨드 시스템(ICommand 등)을 사용하여 실행
    // 예: CommandInvoker::execute(command, context);
    
    // 임시 테스트용 에코 (구조 확인용)
    // ISession* session = findSession(fd);
    // if (session) session->send("Server echoes: " + commandLine + "\r\n");
}