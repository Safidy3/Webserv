
#include "webserv.hpp"

void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		flags = 0;
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void	remove_client(std::vector<pollfd>& pool_fds, int index)
{
	close(pool_fds[index].fd);
	pool_fds.erase(pool_fds.begin() + index);
}

void	add_new_client(std::vector<pollfd>& pool_fds, int new_client_fd)
{
	pollfd	new_client;

	new_client.fd = new_client_fd;
	new_client.events = POLLIN;
	new_client.revents = 0;
	
	set_nonblocking(new_client_fd);
	pool_fds.push_back(new_client);
	std::cout << "New client FD: " << new_client_fd << "\n";
}

/*
	AF_INET		: IPv4 protocol family.
	SOCK_STREAM	: TCP type socket.

	SOL_SOCKET				: Level You're setting a socket-level option
	SO_REUSEADDR 			: Option name, the specific option to enable address reuse
	Pointer to value (1)	: Enables the option
	Size of the value		: Tells how many bytes are in opt

	enables the SO_REUSEADDR option on a socket :
		Allow this socket to bind to a port immediately even if a previous connection on that port is still in TIME_WAIT state.
	When a server program closes a socket, the port may go into a TIME_WAIT state — meaning it can't be reused for a short time

	sockaddr_in	: data type that is used to store the address of the socket.
	INADDR_ANY	: used when we don't want to bind our socket to any particular
		IP and instead make it listen to all the available IPs.
	htons()		: convert the unsigned int from machine byte order to network byte order.
*/

int	add_server(std::vector<pollfd>& pool_fds, int port, int max_con)
{
	int					server_fd;
	int					opt;
	struct sockaddr_in	server_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Socket creation failed\n";
		return (-1);
	}
	set_nonblocking(server_fd);

	opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));

	pollfd	new_serv;
	new_serv.fd = server_fd;
	new_serv.events = POLLIN;
	new_serv.revents = 0;
	pool_fds.push_back(new_serv);

	listen(server_fd, max_con);
	std::cout << "Server running on port " << port << "...\n";
	return (server_fd);
}


std::string float_to_str(float f)
{
    std::stringstream s;
    s << f;
    return s.str();
}

/*
	struct pollfd
	{
		int   fd;         file descriptor
		short events;     requested events : a bit mask specifying the events the application is interested in
		short revents;    returned events : filled by the kernel with the events that actually occurred
	};

	poll(struct pollfd *clients_fds, nfds_t nfds, int timeout) :
		waits for one an array of file descriptors to become ready to perform I/O.

	- timeout :
		specifies the number of milliseconds that poll() should block waiting for a file descriptor to become ready.

	The call will block until either:
		•  a file descriptor becomes ready;
		•  the call is interrupted by a signal handler; or
		•  the timeout expires.
*/
int main()
{
	std::vector<pollfd>		pool_fds;
	std::vector<int>		servers_fds;

	int serv_fd = add_server(pool_fds, 8080, 10);
	if (serv_fd == -1)
	{
		std::cerr << "server error.\n";
		exit(1);
	}
	servers_fds.push_back(serv_fd);
	while (true)
	{
		int	ready = poll(pool_fds.data(), pool_fds.size(), -1);
		if (ready < 0)
		{
			std::cerr << "Error: poll\n";
			break;
		}

		for (size_t i = 0; i < pool_fds.size(); ++i)
		{
			std::vector<int>::iterator it = find(servers_fds.begin(), servers_fds.end(), pool_fds[i].fd);

			if (it != servers_fds.end())
			{
				pollfd	&serv = pool_fds[i];
				int		new_client_fd;

				if (serv.revents & POLLIN)
				{
					new_client_fd = accept(serv.fd, NULL, NULL);
					if (new_client_fd >= 0)
						add_new_client(pool_fds, new_client_fd);
				}
			}
			else
			{
				pollfd	&client = pool_fds[i];if (client.revents & POLLIN)
				{
					char	data_buffer[1024];
					int		bytes_read = read(client.fd, data_buffer, sizeof(data_buffer) - 1);
					if (bytes_read > 0)
					{
						const char*	path = "./static/index.html";
						struct stat	st;
						if (stat(path, &st) != 0)
						{
							// send 404
							const char* notfound = "HTTP/1.0 404 Not Found\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";
							send(client.fd, notfound, strlen(notfound), 0);
							shutdown(client.fd, SHUT_WR);
							remove_client(pool_fds, i); --i;
							continue;
						}

						std::ifstream file(path, std::ios::binary);
						if (!file.is_open())
						{
							const char* err = "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 5\r\nConnection: close\r\n\r\nError";
							send(client.fd, err, strlen(err), 0);
							shutdown(client.fd, SHUT_WR);
							remove_client(pool_fds, i); --i;
							continue;
						}

						std::string header =
							"HTTP/1.0 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Content-Length: " + float_to_str(st.st_size) + "\r\n"
							"Connection: close\r\n\r\n";

						// send header (handle partial send)
						const char*	hdr_ptr = header.data();
						size_t		hdr_left = header.size();

						while (hdr_left > 0)
						{
							ssize_t n = send(client.fd, hdr_ptr, hdr_left, 0);
							if (n > 0)
							{
								hdr_ptr += n;
								hdr_left -= n;
								continue;
							}
							if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
							{
								pollfd p;
								p.fd = client.fd;
								p.events = POLLOUT;
								p.revents = 0;
								poll(&p, 1, -1);
								continue;
							}
							// error -> close
							file.close();
							shutdown(client.fd, SHUT_WR);
							remove_client(pool_fds, i); --i;
							goto continue_outer_loop;
						}

						// send file in chunks using send()
						const size_t		CHUNK = 1024;
						std::vector<char>	buf(CHUNK);

						size_t remaining = st.st_size;
						while (remaining > 0 && file)
						{
							size_t			to_read;
							std::streamsize r;
							const char*		ptr;
							
							to_read = std::min(CHUNK, remaining);
							file.read(buf.data(), to_read);
							r = file.gcount();
							if (r <= 0)
								break;

							ptr = buf.data();
							size_t left = (size_t)r;
							while (left > 0)
							{
								ssize_t s = send(client.fd, ptr, left, 0);
								if (s > 0)
								{
									ptr += s;
									left -= s;
									remaining -= s;
									continue;
								}
								if (s == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
								{
									pollfd p;
									p.fd = client.fd;
									p.events = POLLOUT;
									p.revents = 0;
									poll(&p, 1, -1);
									continue;
								}
								// error
								file.close();
								shutdown(client.fd, SHUT_WR);
								remove_client(pool_fds, i); --i;
								goto continue_outer_loop;
							}
						}
						// done: close connection and remove client
						file.close();
						shutdown(client.fd, SHUT_WR);
						remove_client(pool_fds, i);
						--i;
					}
					else if (bytes_read == 0)
					{
						std::cout << "Client "<< client.fd << " disconected !\n";
						remove_client(pool_fds, i);
						--i;
					}
				}
				continue_outer_loop: ; // label target for goto cleanup
			}
		}
	}

	for (size_t i = 0; i < pool_fds.size(); ++i)
		close(pool_fds[i].fd);
	return 0;
}
