/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:48 by safandri          #+#    #+#             */
/*   Updated: 2025/09/20 15:53:18 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "include/ConfigParser.hpp"
#include "include/Server.hpp"
#include "include/Client.hpp"
#include "include/HTTPRequest.hpp"
#include "include/HTTPResponse.hpp"
#include "include/ServerManager.hpp"


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

static const size_t BUFFER_SIZE = 1024;
static const int MAX_PENDING_QUEUE = 10;
static const int MAX_CLIENTS = 100;

int main(int argc, char **argv)
{
	std::string configPath;
    std::string mimeTypesPath = "./conf.d/mime.type";

    if (argc == 2 && argv[1][0])
        configPath = argv[1];  
    else if (argc == 1)
        configPath = "./conf.d/webserv.conf";
    else
        return (std::cerr << "Use: " << argv[0]
				<< " [config_file]" << std::endl, EXIT_FAILURE);



    try
    {
		ConfigParser	parser(configPath, mimeTypesPath);
		HttpConfig		config = parser.parse();
		MimeTypes		types;
		parser.loadMimeTypes(types);



		ServerManager serverManager;

		for (size_t i = 0; i < config.servers.size(); ++i)
		{
			serverManager.addServer(config.servers[i].listenPort, MAX_CLIENTS);
		}
		
		serverManager.addServer(8080, 10);
		serverManager.pollEvents();
    }
    catch (const std::exception& e)
    {
        return (std::cerr << "Error: " << e.what() << std::endl, EXIT_FAILURE);
    }
    
    return 0;
}