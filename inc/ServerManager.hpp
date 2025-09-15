#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include.hpp"


class ServerManager
{
	private:
		std::vector<pollfd>				_poolFds;
		std::map<int, Server*>			_serversMap; // map of server fd to Server object
		std::map<int, Client*>			_clientsMap; // map of client fd to Client object

	public:
		ServerManager()
		{
			_poolFds.clear();
			_serversMap.clear();
			_clientsMap.clear();
		};
		~ServerManager() {};

		void	addServer(int port, int max_con);
		void	removeServer(int server_fd);
		void	addClient(int client_fd);
		void	removeClient(int client_fd);

		void	pollEvents();
};


void	ServerManager::addServer(int port, int max_con)
{
	Server* new_server = new Server(ServerConfig_t(), port, max_con);
	if (!new_server->init())
	{
		std::cerr << "Failed to initialize server on port " << port << "\n";
		delete new_server;
		return;
	}
	_serversMap[new_server->getSocket()] = new_server;
	_poolFds.push_back(new_server->getPollFD());
	std::cout << "Server added on port " << port << "\n";
}

void	ServerManager::removeServer(int server_fd)
{
	std::map<int, Server*>::iterator it = _serversMap.find(server_fd);
	if (it != _serversMap.end())
	{
		delete it->second;
		_serversMap.erase(it);
		// Also remove from _poolFds
		for (std::vector<pollfd>::iterator p_it = _poolFds.begin(); p_it != _poolFds.end(); ++p_it)
		{
			if (p_it->fd == server_fd)
			{
				_poolFds.erase(p_it);
				break;
			}
		}
		std::cout << "Server on fd " << server_fd << " removed\n";
	}
	else
		std::cerr << "Server fd " << server_fd << " not found\n";
}

void	ServerManager::addClient(int client_fd)
{
	Client* new_client = new Client(client_fd);
	if (new_client->getSocket() >= 0)
	{
		_clientsMap[client_fd] = new_client;
		_poolFds.push_back(new_client->getPollFD());
		std::cout << "Client added with fd " << client_fd << "\n";
	}
	else
	{
		std::cerr << "Failed to add client with fd " << client_fd << "\n";
		delete new_client;
	}
}

void	ServerManager::removeClient(int client_fd)
{
	std::map<int, Client*>::iterator it = _clientsMap.find(client_fd);
	if (it != _clientsMap.end())
	{
		it->second->closeConnection();
		delete it->second;
		_clientsMap.erase(it);
		// Also remove from _poolFds
		for (std::vector<pollfd>::iterator p_it = _poolFds.begin(); p_it != _poolFds.end(); ++p_it)
		{
			if (p_it->fd == client_fd)
			{
				_poolFds.erase(p_it);
				break;
			}
		}
		std::cout << "Client with fd " << client_fd << " removed\n";
	}
	else
		std::cerr << "Client fd " << client_fd << " not found\n";
}

void	ServerManager::pollEvents()
{
	int ready = poll(_poolFds.data(), _poolFds.size(), -1);
	if (ready < 0)
	{
		std::cerr << "Error: poll\n";
		return;
	}

	for (size_t i = 0; i < _poolFds.size(); ++i)
	{

		int fd = _poolFds[i].fd;

		Server *server = _serversMap[fd]; // Check if fd belongs to a server
		Client *client = _clientsMap[fd]; // Or to a client

		if (server)
		{
			if (_poolFds[i].revents & POLLIN)
			{
				int new_client_fd = accept(server->getSocket(), NULL, NULL);
				if (new_client_fd < 0)
				{
					std::cerr << "Error: accept failed (" << strerror(errno) << ")\n";
					continue;
				}
				addClient(new_client_fd);
			}
		}
		else if (client)
		{
			if (_poolFds[i].revents & POLLIN)
			{
				ssize_t bytes_read = client->readData();
				if (bytes_read <= 0)
				{
					removeClient(client->getSocket());
					--i; // Adjust index after removal
				}
				else
				{
					// Process client request and send response
					HTTPResponse response;
					response.setBody("<html><body><h1>Hello, World!</h1></body></html>");
					response.setStatus(200);
					response.setHeader("Content-Type", "text/html");
					response.setHeader("Content-Length", toString(response.getBody().size()));
					response.setHeader("Connection", "close");

					client->sendData(response.toString());

					// After sending response, close connection
					removeClient(client->getSocket());
					--i; // Adjust index after removal
				}
			}
		}
	}
}

#endif