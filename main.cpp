/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:48 by safandri          #+#    #+#             */
/*   Updated: 2025/08/15 10:22:52 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// int main()
// {
// 	std::string str = "GET / HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\n";
// 	std::map<std::string ,std::string> request;
// 	parsString(str, request);
// 	return (0);
// }

// int main()
// {
// 	webserv web;
// 	return (0);
// }


typedef struct	clientDetails
{
	int32_t				clientfd;	// client file descriptor
	int32_t				serverfd;	// server file descriptor
	std::vector<int>	clientList;	// for storing all the client fd
	clientDetails(void)
	{
		// initializing the variable
		this->clientfd=-1;
		this->serverfd=-1;
	}
}	clientDetails;

// const int port=4277;
const int port = 8080;
const char ip[] = "0.0.0.0";
const int backlog = 5; // maximum number of connection allowed


int main()
{
	clientDetails* client = new clientDetails();

	client->serverfd = socket(AF_INET, SOCK_STREAM, 0); // for tcp connection
	if (client->serverfd<=0)
	{
		std::cerr << "socket creation error\n";
		delete client;
		exit(1);
	}
	else
		std::cout<<"socket created\n";

	// setting serverFd to allow multiple connection
	int opt = 1;
	if (setsockopt(client->serverfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof opt) < 0)
	{
		std::cerr<<"setSocketopt error\n";
		delete client;
		exit(2);
	}

	// setting the server address
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddr.sin_addr);
	
	// binding the server address
	if (bind(client->serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cerr << "bind error\n";
		delete client;
		exit(3);
	}
	else
		std::cout << "server binded\n";

	// listening to the port
	if (listen(client->serverfd, backlog) < 0)
	{
		std::cerr << "listen error\n";
		delete client;
		exit(4);
	}
	else
		std::cout << "server is listening\n";

	fd_set	readfds;
	size_t	valread;
	int		maxfd;
	int		sd = 0;
	int		activity;

	while (true)
	{
		std::cout << "waiting for activity\n";
		FD_ZERO(&readfds);
		FD_SET(client->serverfd, &readfds);
		maxfd = client->serverfd;
		// copying the client list to readfds so that we can listen to all the client
		for(int sd:client->clientList)
		{
			FD_SET(sd, &readfds);
			if (sd > maxfd)
				maxfd = sd;
		}
		if (sd > maxfd)
			maxfd = sd;

		/*
		using select for listen to multiple client
			select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict errorfds, struct timeval *restrict timeout);
		*/
		activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (activity < 0)
		{
			std::cerr << "select error\n";
			continue;
		}

		/* if something happen on client->serverfd then it means its new connection request */
		if (FD_ISSET(client->serverfd, &readfds))
		{
			client->clientfd = accept(client->serverfd, (struct sockaddr *) NULL, NULL);
			if (client->clientfd < 0)
			{
				std::cerr << "accept error\n";
				continue;
			}
			// adding client to list
			client->clientList.push_back(client->clientfd);
			std::cout << "new client connected\n";
			std::cout << "new connection, socket fd is " << client->clientfd << ", ip is: " 
				<< inet_ntoa(serverAddr.sin_addr) << ", port: " << ntohs(serverAddr.sin_port) << "\n";
		}
		/*
		* else some io operation on some socket
		*/

		// for storing the recive message
		char	message[1024];
		for(long unsigned int i = 0; i < client->clientList.size(); ++i)
		{
			sd = client->clientList[i];
			if (FD_ISSET(sd, &readfds))
			{
				valread = read(sd, message, 1024);
				//check if client disconnected
				if (valread == 0)
				{
					std::cout << "client disconnected\n";
					// getpeername(sd, (struct sockaddr*)&serverAddr, (socklen_t*)&serverAddr);
					// getpeername name return the address of the client (sd)
					std::cout << "host disconnected, ip: " << inet_ntoa(serverAddr.sin_addr) << ", port: " << ntohs(serverAddr.sin_port)<<"\n";
					close(sd);
					/* remove the client from the list */
					client->clientList.erase(client->clientList.begin() + i);
				}
				else
					std::cout << "message from client: " << message << "\n";
				/*
				* handle the message in new thread
				* so that we can listen to other client
				* in the main thread
				* std::thread t1(handleMessage, client, message);
				* // detach the thread so that it can run independently
				* t1.detach();
				*/
			}
		}
	}
	delete client;
	return 0;
}
