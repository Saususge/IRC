#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Server.hpp"

void gracefulQuit(int sig) {
  (int)sig;
  std::cout << "Terminating the server." << std::endl;
  exit(0);
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
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }
  int port = std::atoi(argv[1]);
  std::string password = argv[2];

  Server server(port, password);
  server.run();
  return 0;
}