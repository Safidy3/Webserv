/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:43 by safandri          #+#    #+#             */
/*   Updated: 2025/08/13 15:17:00 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

webserv::~webserv(){}

webserv::webserv()
{
	serv_listn();
	pars_request();
	send_response();
	end_conex();
}

void	webserv::serv_error(const std::string& str)
{
	std::cerr << "Error ! " << str << std::endl;
	exit(EXIT_FAILURE);
}

void	webserv::serv_listn()
{
	struct sockaddr_in	server_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		serv_error("Socket creation failed !");

	server_addr.sin_family = AF_INET;			// 0.0.0.0
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		serv_error("Bind failed !");

	if (listen(server_fd, 1) < 0)
		serv_error("Listen failed !");

	std::cout << "Server running on port 8080...\n";

	client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0)
		serv_error("Connection failed !");
}

void	webserv::send_response()
{
	const char* http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 100\r\n"
		"\r\n"
		"<h1>!</h1>";
	write(client_fd, http_response, strlen(http_response));
}

void	webserv::end_conex()
{
	close(client_fd);
	close(server_fd);
}

void	webserv::pars_request()
{
	char		request_buffer[1024];
	std::string	str;

	memset(request_buffer, 0, sizeof(request_buffer));
	read(client_fd, request_buffer, sizeof(request_buffer) - 1);
	str = request_buffer;
	std::cout << str << std::endl << std::endl;


	size_t line_end = str.find('\n');
	std::string line = str.substr(0, line_end);
	size_t	key_end = line.find(':');
	if (key_end == std::string::npos)
	{
		std::cout << ": not found !!\n";
		std::cout << line << std::endl;
	}
	else
	{
		std::string key = line.substr(0, key_end);
		std::string value = line.substr(key_end + 1, line.size());
		std::cout << key << " -> " << value << std::endl;
	}





	// parsString(str, request);
	// print_map(request);
}


