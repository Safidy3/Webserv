#include "../include/Server.hpp"

Server::Server(ServerConfig_t config, int port, int max_con) :
	_fd(-1), _port(port), _max_con(max_con), _config(config)
{
	// Initialize address structure
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
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
		return false;
	}

	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

	set_nonblocking(_fd);

	if (bind(_fd, (sockaddr*)&_address, sizeof(_address)) < 0)
	{
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

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

Client*	Server::acceptClient()
{
	int new_client_fd = accept(_fd, NULL, NULL);
	if (new_client_fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error: accept failed (" << strerror(errno) << ")\n";
		return (NULL);
	}
	Client* new_client = new Client(new_client_fd, *this);
	_clients.push_back(new_client);
	return new_client;
}

void	Server::removeClient(Client* client)
{
	if (!client)
		return;
	std::vector<Client *>::iterator it = std::find(_clients.begin(), _clients.end(), client);
	if (it != _clients.end())
		_clients.erase(it);
	else
		std::cerr << "Client not found in server's client list\n";
}

int	Server::getSocket() const
{
	return _fd;
}

pollfd&	Server::getPollFD()
{
	return _pollfd;
}

// ServerConfig_t& Server::getConfig()
// {
// 	return _config;
// }
