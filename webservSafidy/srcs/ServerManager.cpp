#include "../include/ServerManager.hpp"

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

void	ServerManager::addServer(ServerConfig_t config, const MimeTypes& mimeTypes)
{
	Server* new_server = new Server(config, &mimeTypes);
	if (!new_server->init())
	{
		std::cerr << "Failed to initialize server on port " << config.listen_port << "\n";
		delete new_server;
		return;
	}
	_serversMap[new_server->getSocket()] = new_server;
	_poolFds.push_back(new_server->getPollFD());
	new_server->printServer();
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

bool	ServerManager::addClient(Client* new_client)
{
	if (new_client->getSocket() >= 0)
	{
		_clientsMap[new_client->getSocket()] = new_client;
		_poolFds.push_back(new_client->getPollFD());
		return (true);
	}
	std::cerr << "Failed to add client with fd " << new_client->getSocket() << "\n";
	return (false);
}

void	ServerManager::removeClient(Client* client)
{
	if (!client)
		return;
	Server& server = client->getServer();
	server.removeClient(client);
	client->closeConnection();

	std::map<int, Client*>::iterator it = _clientsMap.find(client->getSocket());
	if (it != _clientsMap.end())
	{
		delete it->second;
		_clientsMap.erase(it);
	}
}

void	ServerManager::handleIncomingClient(Server* server, pollfd& poll_fd, int fd)
{
	if (poll_fd.revents & POLLIN)
	{
		Client* new_client = server->acceptClient();
		if (new_client)
			if (addClient(new_client))
				return;
		std::cerr << "Failed to accept new client on server fd " << fd << "\n";
		removeClient(new_client);
	}
}

void	ServerManager::handleClientSocket(Client* client, pollfd& poll_fd)
{
	if (poll_fd.revents & POLLIN)
	{
		ssize_t bytes_read = client->readData();
		if (bytes_read > 0)
		{
			Server&				server = client->getServer();
			HTTPResponse		response;
			HTTPRequestParser	parser;
			HTTPRequest			request;

			parser.parse(client->getRawRequest());

			if (parser.getState() == HTTPRequestParser::STATE_COMPLETE)
				request = parser.getRequest();
			else if (parser.getState() == HTTPRequestParser::STATE_ERROR)
			{
				std::cerr << "Error parsing request: " << parser.getErrorMessage() << "\n";
				response = ResponseFactory::badRequest_400();
				client->sendData(response.build());
				removeClient(client);
				return;
			}

			request.printRequest(false);

			HTTPMethodHandler MethodHandler(request, server);
			response = MethodHandler.generateResponse();

			// response.printResponse();
		
			client->sendData(response.build());
		}
		removeClient(client);
	}
	if (poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL))
		removeClient(client);
}

void ServerManager::pollEvents(bool debug)
{
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
			Client* client = client_it->second;
			Server* server = server_it->second;

			// server socket
			if (server_it != _serversMap.end())
				handleIncomingClient(server, _poolFds[i], fd);
			// client socket
			else if (client_it != _clientsMap.end())
				handleClientSocket(client, _poolFds[i]);
			// Orphaned fd - shouldn't happen but clean it up
			else
			{
				// std::cerr << "Warning: Found orphaned fd " << fd << " in poll array\n";
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
		// std::cerr << "\n--- Poll cycle complete ---\n\n";
		if (debug)
			break;
		continue;
	}
}
