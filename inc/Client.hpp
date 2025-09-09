#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Client
{
private:
	int				fd;
	std::string		buffer_in;		// (raw incoming data)
	std::string		buffer_out;		// (pending response to send)
	HTTPRequest		request;
	HTTPResponse	response;
	enum State { READING, WRITING, CLOSED } state;
public:
	Client(int fd);
	~Client();

	void readData();	// → recv() into buffer_in.
	void writeData();	// → send() buffer_out.
	void closeConnection();
};

Client::Client(int fd) : fd(fd), state(READING)
{
	set_nonblocking(fd);
	std::cout << "Client created with FD: " << fd << "\n";
}

void	Client::closeConnection()
{
	if (state != CLOSED)
	{
		close(fd);
		state = CLOSED;
		std::cout << "Client with FD: " << fd << " connection closed\n";
	}
}

Client::~Client()
{
	std::cout << "Client with FD: " << fd << " closed\n";
}

#endif
