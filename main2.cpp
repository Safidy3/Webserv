
#include "webserv.hpp"

int main()
{
	int					server_fd, new_fd;
	struct sockaddr_in	server_addr;
	fd_set				master_set, read_set;
	int					fd_max;
	char				buffer[1024];

	// 1. Create socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Socket creation failed\n";
		return 1;
	}

	// Allow reuse of port
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 2. Binds the socket to 0.0.0.0:8080, allowing connections on that port.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);
	bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	// 3. Listen for incoming connections, max of 10
	listen(server_fd, 10);
	std::cout << "Server running on port 8080...\n";

	// Initialize fd sets
	FD_ZERO(&master_set);
	FD_SET(server_fd, &master_set);
	fd_max = server_fd;

	while (true)
	{
		read_set = master_set; // copy
		if (select(fd_max + 1, &read_set, NULL, NULL, NULL) < 0)
		{
			std::cerr << "select() error\n";
			break;
		}
		// Loop through all fds
		for (int fd = 0; fd <= fd_max; fd++)
		{
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == server_fd)
				{
					// New connection
					new_fd = accept(server_fd, NULL, NULL);
					if (new_fd >= 0)
					{
						FD_SET(new_fd, &master_set);
						if (new_fd > fd_max) fd_max = new_fd;
						std::cout << "New client connected: FD " << new_fd << "\n";
					}
				}
				else
				{
					memset(buffer, 0, sizeof(buffer));
					int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
					if (bytes_read <= 0)
					{
						std::cout << "Client disconnected: FD " << fd << "\n";
						close(fd);
						FD_CLR(fd, &master_set);
					}
					else
					{
						std::cout << "Request from FD " << fd << ":\n" << buffer << "\n";
						// Send simple HTTP response
						const char* http_response =
							"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/plain\r\n"
							"Content-Length: 12\r\n"
							"\r\n"
							"<h1>hello worlds!</h1>";
						write(fd, http_response, strlen(http_response));
						close(fd); // Close after response (HTTP/1.0 style)
						FD_CLR(fd, &master_set);
					}
				}
			}
		}
	}
	close(server_fd);
	return 0;
}
