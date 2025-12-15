#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib>

void exit_with_error(const std::string &msg)
{
	std::cerr << "Error: " << msg << std::endl;
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	int port = std::atoi(argv[1]);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		exit_with_error("Socket creation failed");

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		exit_with_error("Setsockopt failed");

	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
		exit_with_error("Fcntl failed");

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
		exit_with_error("Bind failed");

	if (listen(server_fd, 10) == -1)
		exit_with_error("Listen failed");

	std::cout << "Server listening on port " << port << std::endl;

	std::vector<struct pollfd> poll_fds;
	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	poll_fds.push_back(server_pollfd);

	while (true)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);

		if (ret == -1)
			exit_with_error("Poll failed");

		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				if (poll_fds[i].fd == server_fd)
				{
					std::cout << "New connection attempt..." << std::endl;
				}
				else
				{
				}
			}
		}
	}

	close(server_fd);
	return 0;
}