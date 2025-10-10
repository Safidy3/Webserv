#include "../include/Client.hpp"

Client::Client(int fd, Server& server) : _fd(fd), _server(server), _state(READING)
{
	// Initialize pollfd first
	_pollfd.fd = _fd;
	_pollfd.events = POLLIN;
	_pollfd.revents = 0;

	// Validate file descriptor
	if (_fd < 0)
	{
		std::cerr << "Invalid file descriptor passed to Client constructor\n";
		_state = CLOSED;
		return;
	}

	// Set non-blocking mode
	set_nonblocking(_fd);
	printClient();
}

Client::~Client()
{
	closeConnection();
	// std::cout << "~Client FD " << _fd << " destroyed\n";
}

ssize_t Client::readData()
{
	if (_state == CLOSED || _fd < 0)
		return -1;

	char tempBuffer[1024];
	ssize_t totalBytesRead = 0;

	// Don't clear buffer - accumulate data for partial HTTP requests

	while (true)
	{
		ssize_t receivedDataLength = recv(_fd, tempBuffer, sizeof(tempBuffer) - 1, MSG_DONTWAIT);

		if (receivedDataLength > 0)
		{
			// Null-terminate and append to buffer
			tempBuffer[receivedDataLength] = '\0';
			_raw_request.append(tempBuffer, receivedDataLength);
			totalBytesRead += receivedDataLength;

			// If we received less than the buffer size, we've read all available data
			if (receivedDataLength < (ssize_t)(sizeof(tempBuffer) - 1))
				break;
		}
		else if (receivedDataLength == 0)
		{
			// Connection closed by peer
			_state = CLOSED;
			return 0;
		}
		else // receivedDataLength < 0
		{
			// No more data available right now
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				// Real error occurred
				std::cerr << "recv() error for client FD " << _fd << ": " << strerror(errno) << "\n";
				_state = CLOSED;
				return -1;
			}
		}
	}

	return totalBytesRead;
}

ssize_t Client::sendData(const std::string &response)
{
	if (_state == CLOSED || _fd < 0)
	{
		std::cerr << "Attempt to send data on closed connection (FD: " << _fd << ")\n";
		return -1;
	}

	if (response.empty())
	{
		std::cerr << "Warning: Attempting to send empty response\n";
		return 0;
	}

	ssize_t totalSent = 0;
	size_t remaining = response.size();
	const char *data = response.c_str();

	// Handle partial sends
	while (remaining > 0)
	{
		ssize_t sentDataLength = send(_fd, data + totalSent, remaining, MSG_NOSIGNAL);

		if (sentDataLength > 0)
		{
			totalSent += sentDataLength;
			remaining -= sentDataLength;
		}
		else if (sentDataLength == 0)
		{
			std::cerr << "Warning: send() returned 0 for client FD " << _fd << "\n";
			break;
		}
		else // sentDataLength < 0
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Socket buffer full, would need to wait or use POLLOUT
				std::cerr << "Send would block for client FD " << _fd << "\n";
				break;
			}
			else if (errno == EPIPE || errno == ECONNRESET)
			{
				std::cerr << "Client FD " << _fd << " disconnected during send\n";
				_state = CLOSED;
				return -1;
			}
			else
			{
				std::cerr << "send() error for client FD " << _fd << ": " << strerror(errno) << "\n";
				_state = CLOSED;
				return -1;
			}
		}
	}

	if (totalSent > 0)
	{
		// std::cout << "Successfully sent " << totalSent << " bytes to client FD " << _fd << "\n";
		_state = WRITING; // Update state after successful send
	}

	return totalSent;
}

void	Client::closeConnection()
{
	// _server.removeClient(this);
	if (_state != CLOSED && _fd >= 0)
	{
		if (close(_fd) != 0)
			std::cerr << "Error closing client FD " << _fd << ": " << strerror(errno) << "\n";
		_state = CLOSED;
		_pollfd.fd = -1; // Mark pollfd as invalid
	}
}

void	Client::printClient() const
{
	sockaddr_in	addr;
	socklen_t	addrlen = sizeof(addr);
	memset(&addr, 0, sizeof(addr));
	
	if (getpeername(_fd, (sockaddr *)&addr, &addrlen) == 0)
		std::cout << "*Client FD: " << _fd << " " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n";
	else
		std::cout << "Client FD: " << _fd << " (could not get peer address: " << strerror(errno) << ")\n";
}
