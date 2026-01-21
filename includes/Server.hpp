#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <poll.h>

#include "Manager.hpp"

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();

		void run();

	private:
		Server(const Server &);
		Server &operator=(const Server &);

		void initSocketOrDie();
		void setNonBlockingOrDie(int fd);
		void closeClient(size_t pollIndex);
		void acceptClients();
		void handleClientReadable(size_t pollIndex);
		void onLine(int fd, const std::string &line);

		int _port;
		std::string _password;
		int _serverFd;

		std::vector<struct pollfd> _pollFds;
		std::map<int, std::string> _inbuf;

		Manager manager;
};

#endif
