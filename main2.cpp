
#include "webserv.hpp"

// Set fd to non-blocking mode
void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		flags = 0;
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void	remove_client(std::vector<pollfd> fds, int index)
{
	close(fds[index].fd);
	fds.erase(fds.begin() + index);
}

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Socket creation failed\n";
		return 1;
	}
	set_nonblocking(server_fd);

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);
	bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
	listen(server_fd, 10);

	std::cout << "Server running on port 8080...\n";

	std::vector<pollfd> fds;
	fds.push_back({server_fd, POLLIN, 0});

	while (true)
	{
		int ready;

		ready = poll(fds.data(), fds.size(), -1);
		if (ready < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				// Check for new client
				if (fds[i].fd == server_fd)
				{
					int new_fd;

					new_fd = accept(server_fd, NULL, NULL);
					if (new_fd >= 0)
					{
						set_nonblocking(new_fd);
						fds.push_back({new_fd, POLLIN, 0});
						std::cout << "New client FD: " << new_fd << "\n";
					}
				}
				// Check for existing client data
				else
				{
					char	data_buffer[1024];
					int		bytes_read;

					memset(data_buffer, 0, sizeof(data_buffer));
					bytes_read = read(fds[i].fd, data_buffer, sizeof(data_buffer) - 1);
					if (bytes_read > 0)
					{
						const char* response =
							"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Content-Length: 15\r\n"
							"\r\n"
							"<h1>hello world!</h1>";
						write(fds[i].fd, response, strlen(response));
					}
					else if (bytes_read == 0)
					{
						// connection closed by peer
						std::cout << "Client "<< fds[i].fd << " disconected !\n";
						remove_client(fds, i);
						--i; // Adjust index after erasing
					}
				}
			}
		}
	}

	close(server_fd);
	return 0;
}
