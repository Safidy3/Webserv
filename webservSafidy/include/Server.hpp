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
	bool	isLocationAutoindexOn(const std::string& path) const;
	bool	isUriValidFile(const std::string& uri) const;
	bool	hasErrorPage(int code) const;

	const std::string					getRoot() const { return _config.root; }
	const LocationConfig_t*				getLocationsConfigFromURI(const std::string& path) const;

	const std::string					getLocationAbsolutePath(const std::string& uri) const;
	const std::string					getLocationRoot(const std::string& path) const;
	const std::string					getErrorPage(int code) const;
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