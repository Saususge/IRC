#include <csignal>
#include <cstring>

#include "Bot.hpp"

sig_atomic_t g_running = true;

void gracefulQuit(int sig) {
  (void)sig;
  std::cout << "Terminating the bot." << std::endl;
  g_running = false;
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
  if (argc < 4 || argc > 5) {
    std::cerr << "Usage: " << argv[0]
              << " <server_ip> <port> <password> [channel]" << std::endl;
    return 1;
  }

  set_signal();
  std::string ip = argv[1];
  int port = std::atoi(argv[2]);
  std::string password = argv[3];
  std::string channel = "#general";
  if (argc == 5) {
    channel = argv[4];
  }

  if (port <= 0 || port > 65535) {
    std::cerr << "Error: invalid port number" << std::endl;
    return 1;
  }

  Bot bot(ip, port, password, channel);
  bot.connectServer();
  bot.start();

  return 0;
}
