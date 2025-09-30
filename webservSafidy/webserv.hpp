/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:39 by safandri          #+#    #+#             */
/*   Updated: 2025/09/20 15:25:41 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
// #include <fcntl.h>		// for non-blocking fd
#include <poll.h>		// for io multiplexing (poll)

#include "utils/utils.hpp"

struct LocationConfig_t
{
	std::string							path;
	std::string							root;
	std::string							returnPath;
	std::string							cgiExtension;
	std::string							cgiPath;
	std::string							uploadDir;
	std::string							defaultFile;
	bool								autoindex;
	int									returnCode;
	std::vector<std::string>			indexFiles;
	std::vector<std::string>			methods;
	std::map<std::string,std::string>	directives;

	LocationConfig_t() : autoindex(false), returnCode(0) {}
};

struct ServerConfig_t
{
	std::string					root;
	std::string					host;
	std::vector<std::string>	indexFiles;
	std::map<int,std::string>	errorPages;
	std::vector<LocationConfig_t>	locations;
	int							listenPort;
	size_t						clientMaxBodySize;
};

void set_nonblocking(int fd);
// bool	parseHttpRequest(const char *rawRequest, HttpRequest &request);



#endif