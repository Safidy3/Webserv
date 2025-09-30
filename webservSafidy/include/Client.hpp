#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Server.hpp"

class Server;
class Client
{
private:
	int				_fd;
	std::string		_buffer_in;
	std::string		_buffer_out;
	pollfd			_pollfd;
	HTTPRequest		_request;
	HTTPResponse	_response;
	Server&			_server;
	enum State { READING, WRITING, CLOSED } _state;

public:
	Client(int fd, Server& server);
	~Client();

	ssize_t readData();
	ssize_t sendData(const std::string& response);
	void	closeConnection();

	Server&			getServer() const { return _server; }
	int				getSocket() const;
	HTTPRequest&	getHTTPRequest();
	pollfd&			getPollFD();
	State			getState() const { return _state; }

	void			setState(State state) { _state = state; }
};

#endif