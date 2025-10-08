#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../webserv.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPMethods.hpp"

class ServerManager
{
private:
	std::vector<pollfd>		_poolFds;
	std::map<int, Server*>	_serversMap; // map of server fd to Server object
	std::map<int, Client*>	_clientsMap; // map of client fd to Client object

public:
	ServerManager() 
	{
		_poolFds.clear();
		_serversMap.clear();
		_clientsMap.clear();
	}

	~ServerManager()
	{
		for (std::map<int, Server*>::iterator it = _serversMap.begin(); it != _serversMap.end(); ++it)
			delete it->second;
		for (std::map<int, Client*>::iterator it = _clientsMap.begin(); it != _clientsMap.end(); ++it)
			delete it->second;
	}

	void	addServer(ServerConfig_t config, const MimeTypes& mimeTypes);
	void	removeServer(int server_fd);
	bool	addClient(Client* new_client);
	void	removeClient(Client* client);

	void    pollEvents(bool debug = false);
	void	handleIncomingClient(Server* server, pollfd& poll_fd, int fd);
	void	handleClientSocket(Client* client, pollfd& poll_fd);

	void	printPool();
};

#endif