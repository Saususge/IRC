#include "Server.hpp"
#include "utils.hpp"
#include "numeric.hpp"

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sstream> 

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
	_outbuf.erase(fd);
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
	std::stringstream ss;

	std::cout << "fd=" << fd << " << " << line << std::endl;	
	std::string command, param;
	ss >> command >> param;
	if (command == "PASS")
	{
		if (param == this->_password)

	}
}

void Server::handleClientReadable(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
	char buf[6974];

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

			if (fd == _serverFd)
			{
				if (_pollFds[i].revents & POLLIN)
					acceptClients();
				++i;
				continue;
			}

			if (_pollFds[i].revents & POLLIN)
				handleClientReadable(i);

			// handleClientReadable() may have closed the client (and erased this poll entry).
			if (i >= _pollFds.size() || _pollFds[i].fd != fd)
				continue;

			if (_pollFds[i].revents & POLLOUT)
				handleClientWritable(i);

			// If the client got closed, the current index now points to the next fd.
			if (i < _pollFds.size() && _pollFds[i].fd == fd)
				++i;
		}
	}
}

void Server::queueMessage(int fd, const std::string &msg)
{
	_outbuf[fd] += msg + "\r\n";
	updatePollEvents(fd);
}

void Server::handleClientWritable(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
    std::cout << "[DEBUG] handleClientWritable fd=" << fd 
              << " bufsize=" << _outbuf[fd].size() << std::endl;

	std::map<int, std::string>::iterator it = _outbuf.find(fd);
	if (it == _outbuf.end() || it->second.empty())
	{
		updatePollEvents(fd);
		return;
	}

	while (!it->second.empty())
	{
		ssize_t n = ::send(fd, it->second.data(), it->second.size(), 0);
		if (n < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			closeClient(pollIndex);
			return;
		}
		if (n == 0)
			break;
		it->second.erase(0, static_cast<std::string::size_type>(n));
	}

	updatePollEvents(fd);
}
void Server::updatePollEvents(int fd)
{
    // 1. fd를 찾기
    size_t i;
    for (i = 0; i < _pollFds.size(); ++i)
    {
        if (_pollFds[i].fd == fd)
            break;
    }
    
    // 2. Guard: 못 찾으면 조기 반환
    if (i == _pollFds.size())
    {
        std::cerr << "[ERROR] fd=" << fd << " not found in _pollFds" << std::endl;
        return;  // 또는 assert(false);
    }
    
    // 3. Happy Path: 정상 로직
    short events = POLLIN;
    std::map<int, std::string>::const_iterator it = _outbuf.find(fd);
    if (it != _outbuf.end() && !it->second.empty())
        events |= POLLOUT;
    _pollFds[i].events = events;
    
    // 4. 성공 로그 (실제로 설정 후)
    std::cout << "[DEBUG] fd=" << fd << " events set to: " 
              << (events & POLLIN ? "POLLIN " : "")
              << (events & POLLOUT ? "POLLOUT" : "") 
              << std::endl;
}
