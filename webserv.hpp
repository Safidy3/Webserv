/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:39 by safandri          #+#    #+#             */
/*   Updated: 2025/08/29 11:22:55 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <algorithm>

#include <iostream>		// for cout/cerr
#include <arpa/inet.h>	// for ip inet_pton()
#include <netinet/in.h>	// for address
#include <sys/select.h>	// for io multiplexing (select)
#include <sys/socket.h>	// for socket
#include <unistd.h> 	// for close()
#include <vector>		// for storing client

#include <sstream>		// for float to string
#include <fstream>		// for static_file manipulation
#include <sys/stat.h>	// for stat()
#include <fcntl.h>		// for non-blocking fd
#include <poll.h>		// for io multiplexing (poll)

class webserv
{
	private:
		int									server_fd, client_fd;
		std::string							http_response;
		std::map<std::string, std::string>	request;
		std::map<std::string, std::string>	response;

	public:
		webserv();
		~webserv();

		void	serv_error(const std::string& str);
		void	serv_listn();
		void	pars_request();
		void	send_response();
		void	end_conex();
};

void	parsString(std::string& str, std::map<std::string, std::string>& dict);
void	insertMap(std::string& line, std::map<std::string, std::string>& dict);
void	print_map(std::map<std::string, std::string>& dict);











