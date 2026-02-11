#include "Bot.hpp"

Bot::Bot(const std::string& ip, int port, const std::string& password,
         const std::string& channel)
    : _serverIp(ip),
      _port(port),
      _password(password),
      _nickname("GreetBot"),
      _channel(channel),
      _socket(-1),
      _isConnected(false) {}

Bot::~Bot() {
  if (_socket >= 0) {
    close(_socket);
  }
}

void Bot::connectServer() {
  // Create socket
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0) {
    std::cerr << "Error: socket creation failed" << std::endl;
    return;
  }

  // Set up server address
  struct sockaddr_in serverAddr;
  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_port);
  serverAddr.sin_addr.s_addr = inet_addr(_serverIp.c_str());

  // Connect
  if (connect(_socket, reinterpret_cast<struct sockaddr*>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    std::cerr << "Error: connection failed" << std::endl;
    close(_socket);
    _socket = -1;
    return;
  }

  // Set non-blocking
  fcntl(_socket, F_SETFL, O_NONBLOCK);

  _isConnected = true;
  std::cout << "Connected to " << _serverIp << ":" << _port << std::endl;

  // Authenticate and join channel
  _authenticate();
}

void Bot::_authenticate() {
  sendMessage("PASS " + _password + "\r\n");
  sendMessage("NICK " + _nickname + "\r\n");
  sendMessage("USER " + _nickname + " 0 * :" + _nickname + "\r\n");
  sendMessage("JOIN " + _channel + "\r\n");
}

void Bot::sendMessage(const std::string& msg) {
  if (_socket < 0) {
    return;
  }
  send(_socket, msg.c_str(), msg.size(), 0);
}

void Bot::start() {
  if (!_isConnected) {
    std::cerr << "Error: not connected" << std::endl;
    return;
  }

  struct pollfd pfd;
  pfd.fd = _socket;
  pfd.events = POLLIN;

  char buf[512];

  while (_isConnected) {
    int ret = poll(&pfd, 1, -1);
    if (ret < 0) {
      std::cerr << "Error: poll failed" << std::endl;
      break;
    }
    if (ret == 0) {
      continue;
    }

    if (pfd.revents & POLLIN) {
      std::memset(buf, 0, sizeof(buf));
      ssize_t bytes = recv(_socket, buf, sizeof(buf) - 1, 0);
      if (bytes <= 0) {
        std::cerr << "Disconnected from server" << std::endl;
        _isConnected = false;
        break;
      }
      _processRecv(std::string(buf, bytes));
    }

    if (pfd.revents & (POLLERR | POLLHUP)) {
      std::cerr << "Connection error" << std::endl;
      _isConnected = false;
      break;
    }
  }
}

void Bot::_processRecv(const std::string& data) {
  _recvBuffer += data;

  std::string::size_type pos;
  while ((pos = _recvBuffer.find("\r\n")) != std::string::npos) {
    std::string line = _recvBuffer.substr(0, pos + 2);
    _recvBuffer = _recvBuffer.substr(pos + 2);

    std::cout << "<< " << line;
    Message msg = Parser::parse(line);
    _handleMessage(msg);
  }
}

void Bot::_handleMessage(const Message& msg) {
  // Respond to PING to stay alive
  if (msg.command == "PING") {
    sendMessage("PONG :" + msg.trailing + "\r\n");
    return;
  }

  // Detect JOIN and greet the user
  if (msg.command == "JOIN") {
    // Extract nickname from prefix (nick!user@host)
    std::string nick = msg.prefix;
    std::string::size_type excl = nick.find('!');
    if (excl != std::string::npos) {
      nick = nick.substr(0, excl);
    }

    // Don't greet ourselves
    if (nick == _nickname) {
      return;
    }

    // Determine channel: could be in params[0] or trailing
    std::string channel;
    if (!msg.params.empty()) {
      channel = msg.params[0];
    } else if (!msg.trailing.empty()) {
      channel = msg.trailing;
    }

    if (!channel.empty()) {
      std::string greet =
          "PRIVMSG " + channel + " :Hello " + nick + "!\r\n";
      sendMessage(greet);
      std::cout << ">> " << greet;
    }
  }
}
