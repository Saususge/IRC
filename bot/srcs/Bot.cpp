#include "Bot.hpp"

#include <cerrno>
#include <csignal>

extern sig_atomic_t g_running;

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
}

void Bot::sendMessage(const std::string& msg) {
  _outBuf.append(msg);
}

void Bot::_flushOutBuf() {
  if (_outBuf.empty() || _socket < 0) return;

  ssize_t n = ::send(_socket, _outBuf.c_str(), _outBuf.size(), 0);
  if (n < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      std::cerr << "Error: send failed" << std::endl;
      _isConnected = false;
    }
    return;
  }
  _outBuf.erase(0, n);
}

std::string Bot::_readLine() {
  std::string::size_type pos;
  while ((pos = _inBuf.find("\r\n")) != std::string::npos) {
    std::string line = _inBuf.substr(0, pos);
    _inBuf.erase(0, pos + 2);
    return line;
  }
  while ((pos = _inBuf.find("\n")) != std::string::npos) {
    std::string line = _inBuf.substr(0, pos);
    if (!line.empty() && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);
    _inBuf.erase(0, pos + 1);
    return line;
  }
  return "";
}

void Bot::start() {
  if (!_isConnected) {
    std::cerr << "Error: not connected" << std::endl;
    return;
  }

  struct pollfd pfd;
  pfd.fd = _socket;

  char buf[BUFFER_SIZE];

  while (_isConnected && g_running) {
    pfd.events = POLLIN;
    if (!_outBuf.empty()) {
      pfd.events |= POLLOUT;
    }

    int ret = poll(&pfd, 1, 1000);
    if (ret < 0) {
      if (!g_running) break;
      std::cerr << "Error: poll failed" << std::endl;
      break;
    }
    if (ret == 0) {
      continue;
    }

    if (pfd.revents & POLLIN) {
      ssize_t bytes = recv(_socket, buf, sizeof(buf), 0);
      if (bytes < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        std::cerr << "Disconnected from server" << std::endl;
        _isConnected = false;
        break;
      }
      if (bytes == 0) {
        std::cerr << "Disconnected from server" << std::endl;
        _isConnected = false;
        break;
      }
      if (bytes > 0) {
        _inBuf.append(buf, bytes);
      }

      std::string line;
      while (!(line = _readLine()).empty()) {
        std::cout << "<< " << line << std::endl;
        Message msg = Parser::parse(line);
        _handleMessage(msg);
      }
    }

    if (pfd.revents & POLLOUT) {
      _flushOutBuf();
    }

    if (pfd.revents & (POLLERR | POLLHUP)) {
      std::cerr << "Connection error" << std::endl;
      _isConnected = false;
      break;
    }
  }
}

void Bot::_handleMessage(const Message& msg) {
  // Wait for 001 (RPL_WELCOME) before joining channel
  if (msg.command == "001") {
    std::cout << "Registered successfully, joining " << _channel << std::endl;
    sendMessage("JOIN " + _channel + "\r\n");
    return;
  }

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
