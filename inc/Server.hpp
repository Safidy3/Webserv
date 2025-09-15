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
	ServerConfig_t&	getConfig();
};

Server::Server(ServerConfig_t config, int port, int max_con) :
	_fd(-1),
	_port(port),
	_max_con(max_con),
	_config(config)	
{
	_pollfd.fd = _fd;
	_pollfd.events = POLLIN | POLLOUT;
	_pollfd.revents = 0;
	std::cout << "Server created on port " << _port << "\n";
	init();
}

Server::~Server()
{
	if (_fd != -1)
		close(_fd);
	std::cout << "Server on port " << _port << " closed\n";
}

bool	Server::init()
{
	// Initialize server socket, bind, and listen
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		return (std::cerr << "Socket creation failed\n", false);
	set_nonblocking(_fd);

	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);
	if (bind(_fd, (sockaddr *)&_address, sizeof(_address)) < 0)
		return (std::cerr << "Bind failed\n", false);
	if (listen(_fd, _max_con) < 0)
		return (std::cerr << "Listen failed\n", false);
	return true;
}

void	Server::stop()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
		std::cout << "Server on port " << _port << " stopped\n";
	}
}

int	Server::getSocket() const
{
	return _fd;
}

ServerConfig_t&	Server::getConfig()
{
	return _config;
}

pollfd&	Server::getPollFD()
{
	return _pollfd;
}

#endif
