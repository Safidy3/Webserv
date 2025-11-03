#include "../include/Server.hpp"

Server::Server(ServerConfig_t config, const MimeTypes* mimeTypes) :
	_fd(-1), _config(config), _mimeTypes(mimeTypes)
{
	// Initialize address structure
	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_config.listen_port);

	// Initialize pollfd
	_pollfd.fd = -1;
	_pollfd.events = POLLIN;
	_pollfd.revents = 0;

	std::vector<LocationConfig_t>::iterator it = _config.locations.begin();
	while (it != _config.locations.end())
	{
		if (it->root.empty())
		{
			if (it->path[0] == '/')
				it->root = _config.root + it->path.substr(1);
			else
				it->root = _config.root + it->path;
		}
		if (it->root[it->root.size() - 1] != '/')
			it->root += '/';
		++it;
	}
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

	if (listen(_fd, DEFAULT_MAX_PENDING_CONNECTIONS) < 0)
	{
		std::cerr << "Listen failed: " << strerror(errno) << std::endl;
		close(_fd);
		_fd = -1;
		return false;
	}

	// Update pollfd
	_pollfd.fd = _fd;
	// std::cout << "Server listening on port " << _port << std::endl;
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


/*=================================================================================================*/


bool	Server::isValidLocation(const std::string& location) const
{
	for (size_t i = 0; i < _config.locations.size(); ++i)
		if (location == _config.locations[i].path)
			return true;
	return false;
}

bool	Server::isValidMethod(const std::string& uri, const std::string& method) const
{
	const LocationConfig_t* Location = getLocationsConfigFromURI(uri);
	if (Location)
	{
		if (!Location->methods.empty())
		{
			for (size_t i = 0; i < Location->methods.size(); ++i)
				if (method == Location->methods[i])
					return true;
			return false;
		}
	}
	return true; // If no methods are specified, all are allowed
};

bool	Server::isValidContentType(const std::string& contentType) const
{
	if (!_mimeTypes)
		return false;
	for (MimeTypes::const_iterator it = _mimeTypes->begin(); it != _mimeTypes->end(); ++it)
		if (it->second == contentType)
			return true;
	return false;
};

bool	Server::isValidUri(const std::string& path) const
{
	if (path.empty() || path[0] != '/')
		return false;

	// 1️⃣ Check if it matches any configured location
	const LocationConfig_t* location = getLocationsConfigFromURI(path);
	if (!location)
		return false;

	// 2️⃣ Build the absolute path from server root
	std::string fullPath = location->root;
	if (fullPath.size() > 1 && fullPath[fullPath.size() - 1] == '/')
		fullPath.erase(fullPath.size() - 1);
	fullPath += path.substr(location->path.length());

	// 3️⃣ Check if the file or directory exists on disk
	struct stat s;
	if (stat(fullPath.c_str(), &s) == 0)
		return true;

	return false;
}

bool	Server::isLocationAutoindexOn(const std::string& uri) const
{
	const LocationConfig_t* location = getLocationsConfigFromURI(uri);
	if (location)
		return location->autoindex;
	return false;
}

bool	Server::isUriValidFile(const std::string& uri) const
{

	std::string fullPath;
	const LocationConfig_t* location = getLocationsConfigFromURI(uri);
	if (!location)
		return false;

	fullPath = location->root;
	if (fullPath.size() > 1 && fullPath[fullPath.size() - 1] == '/')
		fullPath.erase(fullPath.size() - 1);
	fullPath += uri.substr(location->path.length());

	if (ftIsFile(fullPath))
		return true;
	return false;
};

/*=================================================================================================*/

const LocationConfig_t*	Server::getLocationsConfigFromURI(const std::string& uri) const
{
	const LocationConfig_t* bestMatchLocation = NULL;
	size_t bestLen = 0;
	std::vector<LocationConfig_t>::const_iterator it;

	for (it = _config.locations.begin(); it != _config.locations.end(); ++it)
	{
		const std::string& locPath = it->path;

		// Match if URI starts with the location path
		if (uri.find(locPath) == 0)
		{
			// Choose the longest match (most specific)
			if (locPath.length() > bestLen)
			{
				bestMatchLocation = &(*it);
				bestLen = locPath.length();
			}
		}
	}

	if (bestMatchLocation)
		return bestMatchLocation;
	return NULL;
}

const std::string	Server::getLocationRoot(const std::string& uri) const
{
	std::string path = uri;
	if (isUriValidFile(uri))
		path = getParentPath(uri);
	const LocationConfig_t* loc = getLocationsConfigFromURI(path);
	if (loc)
		return loc->root;
	return "";
}

const std::string	Server::getLocationValidIndex(const std::string& locationPath) const
{
	if (isUriValidFile(locationPath))
	{
		if (locationPath[0] == '/')
			return _config.root + locationPath.substr(1);
		else
			return _config.root + locationPath;
	}

	std::string rootPath = getLocationRoot(locationPath);
	const LocationConfig_t* loc = getLocationsConfigFromURI(locationPath);
	if (!loc)
		return "";
	for (size_t i = 0; i < loc->index.size(); i++)
	{
		std::string fullPath = rootPath + loc->index[i];
		if (ftFileExists(fullPath))
			return fullPath;
	}
	return "";
}

const std::string	Server::getAbsolutePath(const std::string& uri) const
{
	// 1️⃣ Check if it matches any configured location
	const LocationConfig_t* location = getLocationsConfigFromURI(uri);
	if (!location)
		return "";

	// 2️⃣ Build the absolute path from server root
	std::string fullPath = location->root;
	if (fullPath.size() > 1 && fullPath[fullPath.size() - 1] == '/')
		fullPath.erase(fullPath.size() - 1);
	fullPath += uri.substr(location->path.length());
	std::cout << "\n *** PATH 2 : " << fullPath << std::endl;
	return fullPath;
}

const std::string*	Server::getErrorPage(int code) const
{
	std::map<int, std::string>::const_iterator it = _config.error_pages.find(code);
	if (it != _config.error_pages.end())
		return &it->second;
	return NULL;
}

/*=================================================================================================*/


void	Server::printServer()
{
	ServerConfig_t server = getConfig();
	std::cout << "\nServer: " << server.server_name << std::endl;
	std::cout << "\tListen: " << server.listen_address << ":"
			<< server.listen_port << std::endl;
	if (!server.server_name.empty())
		std::cout << "\tServer Name: " << server.server_name << std::endl;
	if (!server.root.empty())
		std::cout << "\tRoot: " << server.root << std::endl;
	if (!server.log_path.empty())
		std::cout << "\tLog Path: " << server.log_path << std::endl;
	if (!server.index.empty())
	{
		std::cout << "\tIndex: ";
		for (size_t i = 0; i < server.index.size(); i++)
		{
			std::cout << server.index[i];
			if (i < server.index.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "\tClient Max Body Size: " << server.client_max_body_size << std::endl;
	std::cout << "\tKeepalive Timeout: " << server.keepalive_timeout << std::endl;

	if (!server.error_pages.empty())
	{
		std::cout << "\tError Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator it = server.error_pages.begin();
			it != server.error_pages.end(); ++it)
			std::cout << "\t  " << it->first << " -> " << it->second << std::endl;
	}

	if (!server.locations.empty())
	{
		std::cout << "Locations: " << server.locations.size() << std::endl;
		for (size_t i = 0; i < server.locations.size(); i++)
			printLocation(server.locations[i]);
	}
}

void	Server::printLocation(const LocationConfig_t &location)
{
	std::cout << "\tLocation: " << location.path << std::endl;
	std::cout << "\t  Path     : " << location.path << std::endl;
	if (!location.root.empty())
		std::cout << "\t  Root     : " << location.root << std::endl;
	if (!location.index.empty())
	{
		std::cout << "\t  Index    : ";
		for (size_t i = 0; i < location.index.size(); i++)
		{
			std::cout << location.index[i];
			if (i < location.index.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	if (!location.methods.empty())
	{
		std::cout << "\t  Methods  : ";
		for (size_t i = 0; i < location.methods.size(); i++)
		{
			std::cout << location.methods[i];
			if (i < location.methods.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "\t  Autoindex: " << (location.autoindex ? "on" : "off") << std::endl;
	if (location.redirect_code != 0)
		std::cout << "\t  Redirect: " << location.redirect_code << " "
				<< location.redirect_url << std::endl;
	if (!location.cgi_extension.empty())
		std::cout << "\t  CGI Extension: " << location.cgi_extension << std::endl;
	if (!location.cgi_path.empty())
		std::cout << "\t  CGI Path : " << location.cgi_path << std::endl;
	std::cout << std::endl;
}
