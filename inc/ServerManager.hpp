#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include.hpp"


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
		// Clean up all servers and clients
		for (std::map<int, Server*>::iterator it = _serversMap.begin(); it != _serversMap.end(); ++it)
			delete it->second;
		for (std::map<int, Client*>::iterator it = _clientsMap.begin(); it != _clientsMap.end(); ++it)
			delete it->second;
	}

	void    addServer(int port, int max_con);
	void    removeServer(int server_fd);
	void    addClient(int client_fd);
	void    removeClient(int client_fd);
	
	void    pollEvents();
	int		handleIncomingClient(Server* server, pollfd& poll_fd, int fd);
	void	handleClientSocket(Client* client, pollfd& poll_fd);

	void	printPool();
};

void	ServerManager::printPool() 
{
	for (size_t i = 0; i < _poolFds.size(); ++i)
	{
		std::map<int, Server*>::iterator server_it = _serversMap.find(_poolFds[i].fd);
		std::map<int, Client*>::iterator client_it = _clientsMap.find(_poolFds[i].fd);

		if (server_it != _serversMap.end())
			std::cout << "[Server] ";
		else if (client_it != _clientsMap.end())
			std::cout << "[Client] ";
		else
			std::cout << "[Unknown] ";
		std::cout << "FD: " << _poolFds[i].fd 
					<< " Events: " << _poolFds[i].events 
					<< " Revents: " << _poolFds[i].revents << "\n";
	}
}

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
	// std::cout << "Server added on port " << port << "\n";
}
void	ServerManager::removeServer(int server_fd)
{
	std::map<int, Server*>::iterator it = _serversMap.find(server_fd);
	if (it != _serversMap.end())
	{
		delete it->second;
		_serversMap.erase(it);
		
		// Remove from _poolFds
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
		// std::cout << "Client added with fd " << client_fd << "\n";
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
		
		// Remove from _poolFds
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

int		ServerManager::handleIncomingClient(Server* server, pollfd& poll_fd, int fd)
{
	if (poll_fd.revents & POLLIN)
	{
		printPool();
		int new_client_fd = accept(server->getSocket(), NULL, NULL);
		if (new_client_fd < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				std::cerr << "Error: accept failed (" << strerror(errno) << ")\n";
			return (1);
		}
		addClient(new_client_fd);
	}
	if (poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		std::cerr << "Error on server socket " << fd << "\n";
		removeServer(fd);
	}
	return (0);
}

void	ServerManager::handleClientSocket(Client* client, pollfd& poll_fd)
{
	bool should_close = false;

	if (poll_fd.revents & POLLIN)
	{
		ssize_t bytes_read = client->readData();
		if (bytes_read <= 0)
			should_close = true;
		else
		{
			// Process client request and send response
			HTTPResponse response;
			response.setBody("<html><body><h1>Hello, World!</h1></body></html>");
			response.setStatus(200);
			response.setHeader("Content-Type", "text/html");
			response.setHeader("Content-Length", toString(response.getBody().size()));
			response.setHeader("Connection", "close");

			if (client->sendData(response.toString()) < 0)
				should_close = true;
			else
				should_close = true; // Close after sending response
		}
	}

	if (poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL))
		should_close = true;
	// No need to adjust index when iterating backwards
	if (should_close)
		removeClient(client->getSocket());

}

void ServerManager::pollEvents()
{
	// Don't call startServers() here - servers should already be initialized
	if (_poolFds.empty())
	{
		std::cerr << "No file descriptors to poll\n";
		return;
	}

	while (true)
	{
		int ready = poll(_poolFds.data(), _poolFds.size(), -1);
		if (ready <= 0)
		{
			std::cerr << "Error: poll failed: " << strerror(errno) << "\n";
			return;
		}

		// Process events - iterate backwards to handle removals safely
		for (int i = static_cast<int>(_poolFds.size()) - 1; i >= 0; --i)
		{
			if (_poolFds[i].revents == 0)
				continue; // No events for this fd

			// Use safer map lookups
			int fd = _poolFds[i].fd;
			std::map<int, Server*>::iterator server_it = _serversMap.find(fd);
			std::map<int, Client*>::iterator client_it = _clientsMap.find(fd);

			// server socket
			if (server_it != _serversMap.end())
			{
				if (handleIncomingClient(server_it->second, _poolFds[i], fd) == 1)
					continue;
			}
			// client socket
			else if (client_it != _clientsMap.end())
			{
				Client* client = client_it->second;
				handleClientSocket(client, _poolFds[i]);
			}
			else
			{
				// Orphaned fd - shouldn't happen but clean it up
				std::cerr << "Warning: Found orphaned fd " << fd << " in poll array\n";
				for (std::vector<pollfd>::iterator p_it = _poolFds.begin(); p_it != _poolFds.end(); ++p_it)
				{
					if (p_it->fd == fd)
					{
						_poolFds.erase(p_it);
						break;
					}
				}
			}
		}
	}
}

#endif