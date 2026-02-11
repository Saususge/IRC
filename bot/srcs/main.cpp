#include "Bot.hpp"

int main(int argc, char** argv) {
  if (argc < 4 || argc > 5) {
    std::cerr << "Usage: " << argv[0]
              << " <server_ip> <port> <password> [channel]" << std::endl;
    return 1;
  }

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
