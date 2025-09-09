#ifndef SERVER_HPP
#define SERVER_HPP

#include "../webserv.hpp"

class Server
{
private:
	int					serv_fd;
	int					serv_port;
	struct sockaddr_in	serv_address;
	ServerConfig		serv_config;
public:
	Server(ServerConfig config, int port = 8080);
	~Server();

	bool			init(); // → socket(), bind(), listen().
	int				getSocket() const;
	ServerConfig&	getConfig();
};

Server::Server(ServerConfig config, int port) :
	serv_fd(-1),
	serv_port(port),
	serv_config(config)
{
	std::cout << "Server created on port " << serv_port << "\n";
}

Server::~Server()
{
	if (serv_fd != -1)
		close(serv_fd);
	std::cout << "Server on port " << serv_port << " closed\n";
}

bool	Server::init()
{
	// Initialize server socket, bind, and listen
	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_fd < 0)
		return (std::cerr << "Socket creation failed\n", false);
	set_nonblocking(serv_fd);

	int opt = 1;
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	serv_address.sin_family = AF_INET;
	serv_address.sin_addr.s_addr = INADDR_ANY;
	serv_address.sin_port = htons(serv_port);
	if (bind(serv_fd, (sockaddr *)&serv_address, sizeof(serv_address)) < 0)
		return (std::cerr << "Bind failed\n", false);
	if (listen(serv_fd, SOMAXCONN) < 0)
		return (std::cerr << "Listen failed\n", false);
	return true;
}

int	Server::getSocket() const
{
	return serv_fd;
}

ServerConfig&	Server::getConfig()
{
	return serv_config;
}

#endif
