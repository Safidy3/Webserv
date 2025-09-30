#ifndef SERVER_HPP
#define SERVER_HPP

#include "../webserv.hpp"
#include "Client.hpp"

class Client;
class Server
{
private:
	int						_fd;
	int						_port;
	int						_max_con;
	struct sockaddr_in		_address;
	ServerConfig_t&			_config;
	pollfd					_pollfd;
	std::vector<Client *>	_clients;

public:
	Server(ServerConfig_t config, int port = 8080, int max_con = 10);
	~Server();

	void			stop();
	bool			init();

	Client*			acceptClient();
	void			removeClient(Client* client);

	int				getSocket() const;
	pollfd&			getPollFD();
	ServerConfig_t	getConfig();
};

#endif // SERVER_HPP