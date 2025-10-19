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
#include <ctime>
#include <cctype>
#include <filesystem>

#include <iostream>		// for cout/cerr
#include <arpa/inet.h>	// for ip inet_pton()
#include <netinet/in.h>	// for address
#include <sys/select.h>	// for io multiplexing (select)
#include <sys/socket.h>	// for socket
#include <sys/wait.h>	// for waitpid()
#include <unistd.h> 	// for close()
#include <vector>		// for storing client
#include <iterator>

#include <sstream>		// for float to string
#include <fstream>		// for static_file manipulation
#include <sys/stat.h>	// for stat()
#include <fcntl.h>		// for non-blocking fd
#include <poll.h>		// for io multiplexing (poll)

#include "utils/utils.hpp"

static const size_t	BUFFER_SIZE = 1024;
static const int	DEFAULT_MAX_PENDING_CONNECTIONS = 10;
static const int	DEFAULT_DEFAULT_MAX_CONNECTIONS = 100;
static const int	DEFAULT_TIMEOUT = 30;
static const int	DEFAULT_KEEPALIVE_TIMEOUT = 30;
static const size_t	DEFAULT_CLIENT_MAX_BODY_SIZE = 1000000; // 1MB

typedef std::map<std::string, std::string> MimeTypes;

// Location configuration structure
struct LocationConfig_t
{
	std::string					path;
	std::string					root;
	std::vector<std::string>	index;
	std::vector<std::string>	methods;
	bool						autoindex;
	std::string					redirect_url;
	int							redirect_code;
	std::string					cgi_extension;
	std::string					cgi_path;

	LocationConfig_t(): autoindex(false),
						redirect_code(0) {}
};

// Server configuration structure
struct ServerConfig_t
{
	std::string						listen_address;
	int								listen_port;
	std::string						server_name;
	std::string						root;
	std::string						log_path;
	std::vector<std::string>		index;
	size_t							client_max_body_size;
	std::map<int, std::string>		error_pages;
	int								keepalive_timeout;
	std::vector<LocationConfig_t>	locations;

	ServerConfig_t() :  listen_address("127.0.0.1"),
						listen_port(80),
						client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE), 
						keepalive_timeout(DEFAULT_KEEPALIVE_TIMEOUT){}
};
typedef std::vector<ServerConfig_t> ServersConfig_t;
typedef std::vector<ServerConfig_t> ServersConfig;


struct CGIResult
{
	int			exitCode;
	std::string	bodyOutput; // Only the body from CGI script
	std::string	errors;
	bool		timedOut;

	CGIResult() : exitCode(-1), timedOut(false) {}
};

struct Data
{
	std::string	name;
	std::string	age;
	std::string	comment;
};

typedef std::map<int, Data*> DataMap;

void set_nonblocking(int fd);
// bool	parseHttpRequest(const char *rawRequest, HttpRequest &request);

#endif
