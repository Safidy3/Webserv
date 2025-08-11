/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:48 by safandri          #+#    #+#             */
/*   Updated: 2025/08/11 16:39:09 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.h"

// int main()
// {
// 	int					server_fd, client_fd;

// 	server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (server_fd == -1)
// 	{
// 		std::cerr << "Socket creation failed\n";
// 		return 1;
// 	}

// 	// 2. Bind address
// 	struct sockaddr_in	server_addr;
// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
// 	server_addr.sin_port = htons(8080);       // port 8080

// 	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
// 	{
// 		std::cerr << "Bind failed\n";
// 		return 1;
// 	}

// 	// 3. Listen
// 	if (listen(server_fd, 1) < 0)
// 	{
// 		std::cerr << "Listen failed\n";
// 		return 1;
// 	}

// 	std::cout << "Server running on port 8080...\n";

// 	// 4. Accept connection
// 	client_fd = accept(server_fd, NULL, NULL);
// 	if (client_fd < 0)
// 	{
// 		std::cerr << "Accept failed\n";
// 		return 1;
// 	}

// 	// 5. Read HTTP request
// 	char				buffer[1024];

// 	memset(buffer, 0, sizeof(buffer));
// 	read(client_fd, buffer, sizeof(buffer) - 1);
// 	std::cout << "Request:\n" << buffer << std::endl;

//	 6. Send HTTP response
//	 const char* http_response =
//	 	"HTTP/1.1 200 OK\r\n"
//	 	"Content-Type: text/html\r\n"
//	 	"Content-Length: 100\r\n"
//	 	"\r\n"
//	 	"<h1>Hello, world!</h1>";
//	 write(client_fd, http_response, strlen(http_response));

// 	7. Clean up
// 	close(client_fd);
// 	close(server_fd);

// 	return 0;
// }


int main()
{
	webserv web;

	return (0);
}