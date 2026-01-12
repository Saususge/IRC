#include "Server.hpp"
#include "utils.hpp"

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverFd(-1), _pollFds(), _inbuf()
{
	initSocketOrDie();
}

Server::~Server()
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
		::close(_pollFds[i].fd);
	_pollFds.clear();
	_inbuf.clear();
}

void Server::setNonBlockingOrDie(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		exit_with_error("Fcntl failed");
}

void Server::initSocketOrDie()
{
	_serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		exit_with_error("Socket creation failed");

	int opt = 1;
	if (::setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		exit_with_error("Setsockopt failed");

	setNonBlockingOrDie(_serverFd);

	sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);

	if (::bind(_serverFd, (struct sockaddr *)&address, sizeof(address)) == -1)
		exit_with_error("Bind failed");
	if (::listen(_serverFd, 10) == -1)
		exit_with_error("Listen failed");

	pollfd serverPoll;
	serverPoll.fd = _serverFd;
	serverPoll.events = POLLIN;
	serverPoll.revents = 0;
	_pollFds.push_back(serverPoll);
}

void Server::closeClient(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
	_inbuf.erase(fd);
	::close(fd);
	_pollFds.erase(_pollFds.begin() + pollIndex);
}

void Server::acceptClients()
{
	while (true)
	{
		sockaddr_in clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		int clientFd = ::accept(_serverFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			exit_with_error(std::string("accept failed: ") + std::strerror(errno));
		}

		setNonBlockingOrDie(clientFd);

		pollfd clientPoll;
		clientPoll.fd = clientFd;
		clientPoll.events = POLLIN;
		clientPoll.revents = 0;
		_pollFds.push_back(clientPoll);
		_inbuf[clientFd] = "";

		std::cout << "Accepted client fd=" << clientFd << std::endl;
	}
}

void Server::onLine(int fd, const std::string &line)
{
	// Starter behavior: log. Next step is IRC parsing (PASS/NICK/USER/etc.).
	std::cout << "fd=" << fd << " << " << line << std::endl;
}

void Server::handleClientReadable(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
	char buf[4096];

	while (true)
	{
		ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
		if (n == 0)
		{
			closeClient(pollIndex);
			return;
		}
		if (n < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			closeClient(pollIndex);
			return;
		}

		_inbuf[fd].append(buf, n);

		// Extract complete lines. RFC uses CRLF, but accept LF too.
		while (true)
		{
			std::string &acc = _inbuf[fd];
			std::string::size_type lf = acc.find('\n');
			if (lf == std::string::npos)
				break;

			std::string line = acc.substr(0, lf);
			acc.erase(0, lf + 1);
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			onLine(fd, line);
		}
	}
}

void Server::run()
{
	std::cout << "Server listening on port " << _port << std::endl;

	while (true)
	{
		int ret = ::poll(&_pollFds[0], _pollFds.size(), -1);
		if (ret == -1)
			exit_with_error("Poll failed");

		for (size_t i = 0; i < _pollFds.size();)
		{
			int fd = _pollFds[i].fd;

			if (_pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (fd == _serverFd)
					exit_with_error("Server socket error");
				closeClient(i);
				continue;
			}

			if (!(_pollFds[i].revents & POLLIN))
			{
				++i;
				continue;
			}

			if (fd == _serverFd)
			{
				acceptClients();
				++i;
				continue;
			}

			handleClientReadable(i);

			// If the client got closed, the current index now points to the next fd.
			if (i < _pollFds.size() && _pollFds[i].fd == fd)
				++i;
		}
	}
}
