#ifndef SERVER_HPP
#define SERVER_HPP

#include "../webserv.hpp"
#include "Client.hpp"

class Client;
class Server
{
private:
	int						_fd;
	ServerConfig_t			_config;
	struct sockaddr_in		_address;
	pollfd					_pollfd;
	std::vector<Client *>	_clients;
	const MimeTypes*		_mimeTypes;


	std::string getParentPath(const std::string& fullPath) const
	{
		size_t lastSlash = fullPath.find_last_of("/\\"); // Handles both '/' and '\'
		if (lastSlash == std::string::npos)
			return "";
		std::string parentPath = fullPath.substr(0, lastSlash);
		if (parentPath.empty())
			return "/";
		return parentPath;
	}

public:
	Server(ServerConfig_t config, const MimeTypes* mimeTypes = NULL);
	~Server();

	void	stop();
	bool	init();

	Client*	acceptClient();
	void	removeClient(Client* client);

	/*
		root      : /www
		location  : /html
		uri       : /html/index.html || /html
	*/

	bool	isValidLocation(const std::string& location) const;
	bool	isValidMethod(const std::string& path, const std::string& method) const;
	bool	isValidUri(const std::string& path) const;
	bool	isValidContentType(const std::string& contentType) const;
	bool	isUriValidFile(const std::string& uri) const
	{
		std::string fullPath;

		for (std::vector<LocationConfig_t>::const_iterator it = _config.locations.begin(); it != _config.locations.end(); ++it)
		{
			if (it->path == "/")
				fullPath = _config.root + uri.substr(1);
			else if (uri.find(it->path) == 0) // uri starts with location path
				fullPath = it->root + uri.substr(it->path.length());
			else
				continue;
			if (ftIsFile(fullPath))
				break;
		}
		if (ftIsFile(fullPath))
		{
			// std::cout << "File found: " << fullPath << std::endl;
			std::string location = getParentPath(uri);
			if (isValidLocation(location))
			{
				// std::cout << "Valid location found: " << location << std::endl;
				return true;
			}
		}
		// std::cout << "Location not found for URI: " << uri << std::endl << std::endl;
		return false;
	};

	const std::string					getRoot() const { return _config.root; }
	const LocationConfig_t*				getLocationsConfig(const std::string& path) const;
	const std::vector<std::string>*		getLocationMethods(const std::string& path) const;
	bool								getLocationAutoindex(const std::string& path) const;
	
	const std::string					getLocationRoot(const std::string& path) const;
	const std::string*					getErrorPage(int code) const;
	const std::vector<std::string>*		getIndexFiles() const { return &_config.index; };
	const std::string					getLocationValidIndex(const std::string& locationPath) const;

	int									getSocket() const { return _fd; }
	pollfd&								getPollFD() { return _pollfd; }
	ServerConfig_t						getConfig() const { return _config; }
	int									getPort() const { return _config.listen_port; }
	size_t								getClientCount() const { return _clients.size(); }
	std::string							getName() const { return _config.server_name; };

	void 							printServer();
	void							printLocation(const LocationConfig_t &location);

};

#endif