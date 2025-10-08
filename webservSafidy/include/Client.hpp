#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../webserv.hpp"
#include "HTTPResponse.hpp"
#include "Server.hpp"

class Server;
class Client
{
private:
	int				_fd;
	std::string		_raw_request;
	std::string		_buffer_out;
	pollfd			_pollfd;
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
	int				getSocket() const { return _fd; }
	State			getState() const { return _state; }
	std::string&	getRawRequest() { return _raw_request; }
	pollfd&			getPollFD() { return _pollfd; }

	void			setState(State state) { _state = state; }

	void			printClient() const;
};

#endif