#ifndef SERVER_HPP
#define SERVER_HPP

#include "../webserv.hpp"

class Server
{
private:
	int					_fd;
	int					_port;
	int					_max_con;
	struct sockaddr_in	_address;
	ServerConfig_t		_config;
	pollfd				_pollfd;

public:
	Server(ServerConfig_t config, int port = 8080, int max_con = 10);
	~Server();

	void			stop();
	bool			init(); // → socket(), bind(), listen().
	int				getSocket() const;
	pollfd&			getPollFD();
	ServerConfig_t	getConfig();
};

Server::Server(ServerConfig_t config, int port, int max_con)
	: _fd(-1), _port(port), _max_con(max_con), _config(config)
{
	// Initialize address structure properly
	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	// Initialize pollfd
	_pollfd.fd = -1;
	_pollfd.events = POLLIN;
	_pollfd.revents = 0;
}

Server::~Server()
{
	stop();
}

void Server::stop()
{
	if (_fd >= 0)
	{
		close(_fd);
		_fd = -1;
	}
}

bool Server::init()
{
	// Create socket
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
		return false;
	}

	// Set socket options
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

	// Set non-blocking
	set_nonblocking(_fd);

	// Bind socket
	if (bind(_fd, (sockaddr*)&_address, sizeof(_address)) < 0)
	{
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

	// Listen
	if (listen(_fd, _max_con) < 0)
	{
		std::cerr << "Listen failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

	// Update pollfd
	_pollfd.fd = _fd;

	std::cout << "Server listening on port " << _port << std::endl;
	return true;
}

int Server::getSocket() const
{
	return _fd;
}

pollfd& Server::getPollFD()
{
	return _pollfd;
}

// ServerConfig_t& Server::getConfig()
// {
// 	return _config;
// }

#endif // SERVER_HPP