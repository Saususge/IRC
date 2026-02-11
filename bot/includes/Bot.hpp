#ifndef BOT_HPP
#define BOT_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Parser.hpp"

class Bot {
 private:
  // Connection config
  std::string _serverIp;
  int _port;
  std::string _password;
  std::string _nickname;
  std::string _channel;

  int _socket;
  bool _isConnected;

  std::string _inBuf;
  std::string _outBuf;

  static const int BUFFER_SIZE = 512;

 private:
  void _authenticate();
  std::string _readLine();
  void _flushOutBuf();
  void _handleMessage(const Message& msg);

  Bot();
  Bot(const Bot& src);
  Bot& operator=(const Bot& src);

 public:
  Bot(const std::string& ip, int port, const std::string& password,
      const std::string& channel);
  ~Bot();

  void connectServer();
  void start();

  void sendMessage(const std::string& msg);
};

#endif