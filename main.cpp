#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>

#include "Server.hpp"

sig_atomic_t running = true;

void gracefulQuit(int sig) {
  (void)sig;
  std::cout << "Terminating the server." << std::endl;
  running = false;
}

void set_signal(void) {
  struct sigaction sa;

  std::memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = &gracefulQuit;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    std::cerr << "sigaction: SIGINT" << std::endl;
    return;
  }

  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    std::cerr << "sigaction: SIGQUIT" << std::endl;
    return;
  }

  // Ignore SIGPIPE to prevent crash when sending to disconnected clients
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL) == -1) {
    std::cerr << "sigaction: SIGPIPE" << std::endl;
    return;
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }
  set_signal();
  int port = std::atoi(argv[1]);
  if (port <= 0 || port > 65535) {
    std::cerr << "Error: invalid port number (1-65535)" << std::endl;
    return 1;
  }
  std::string password = argv[2];

  Server server(port, password);
  server.run();
  return 0;
}