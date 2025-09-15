#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Client
{
private:
	int				_fd;
	std::string		_buffer_in;
	std::string		_buffer_out;
	pollfd			_pollfd;
	HTTPRequest		_request;
	HTTPResponse	_response;
	enum State { READING, WRITING, CLOSED } state;
public:
	Client(int fd);
	~Client();

	ssize_t	readData();
	ssize_t	sendData(const std::string& response);
	void	closeConnection();

	int				getSocket() const;
	HTTPRequest&	getHTTPRequest();
	pollfd&			getPollFD();
};

Client::Client(int fd) : _fd(fd)
{
	set_nonblocking(_fd);
	state = READING;
	_pollfd.fd = _fd;
	_pollfd.events = POLLIN;
	_pollfd.revents = 0;

	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(_fd, (sockaddr *)&addr, &addrlen);
	std::cout << "New client: " << inet_ntoa(addr.sin_addr) 
				<< ":" << ntohs(addr.sin_port)
				<< " FD: " << _fd << "\n";
}

/// Receive data from client socket
/// - Reads into temp buffer using non-blocking recv()
/// - Appends to internal _buffer
/// - Stops on error or when less than buffer size received
ssize_t Client::readData()
{
    char tempBuffer[8192];
    this->_buffer_in.clear();
    while (true)
	{
        // MSG_DONTWAIT ensures non-blocking mode of recv even if there is no data yet
        // if there is no data, recv() will return -1 and errno will be EWOULDBLOCK or EAGAIN
        ssize_t receivedDataLength = recv(this->_fd, tempBuffer, sizeof(tempBuffer) - 1, MSG_DONTWAIT);
        // If recv returns 0 (connection closed) or -1 (error), stop reading
        if (receivedDataLength <= 0) break;
        // Null-terminate the buffer
        tempBuffer[receivedDataLength] = '\0';
        // Append received data to internal buffer
        this->_buffer_in.append(tempBuffer, receivedDataLength);
        // If less than full buffer was read — assume we're done for now
        if (receivedDataLength < (ssize_t)(sizeof(tempBuffer) - 1)) break;
    }
	// _request.parseHttpRequest(this->_buffer_in.c_str());
	if (!this->_buffer_in.empty())
		_request.parseHttpRequest(this->_buffer_in.c_str());
    return this->_buffer_in.size();
}

ssize_t	Client::sendData(const std::string& response)
{
    ssize_t sentDataLength = send(this->_fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    if (sentDataLength < 0)
        std::cerr << "Error sending data to client" << std::endl;
    else if (sentDataLength == 0)
        std::cerr << "Warning: send() returned 0, no data was sent." << std::endl;
    else
        std::cout << "Successfully sent: " << sentDataLength << " bytes" << std::endl;
    return (sentDataLength);
}

int	Client::getSocket() const
{
	return _fd;
}

HTTPRequest&	Client::getHTTPRequest()
{
	return _request;
}

pollfd&	Client::getPollFD()
{
	return _pollfd;
}

void	Client::closeConnection()
{
	if (state != CLOSED)
	{
		close(_fd);
		state = CLOSED;
		std::cout << "Client with FD: " << _fd << " connection closed\n";
	}
}

Client::~Client()
{
	std::cout << "Client with FD: " << _fd << " closed\n";
}

#endif
