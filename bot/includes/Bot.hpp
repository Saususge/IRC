#ifndef BOT_HPP
#define BOT_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "Parser.hpp"

class Bot {
 private:
  // 1. 연결 정보 (Config)
  std::string _serverIp;
  int _port;
  std::string _password;
  std::string _nickname;

  int _socket;
  bool _isConnected;

  std::string _recvBuffer;

 private:
  void _authenticate();
  void _processRecv(const std::string& data);
  void _handleMessage(const Message& msg);

  Bot();
  Bot(const Bot& src);
  Bot& operator=(const B ot& src);

 public:
  Bot(std::string ip, int port, std::string password);
  ~Bot();

  void connectServer();
  void start();

  void sendMessage(std::string msg);
};

#endif