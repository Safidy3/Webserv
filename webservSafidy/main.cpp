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

static const size_t	BUFFER_SIZE = 1024;
static const int	MAX_PENDING_QUEUE = 10;
static const int	MAX_CLIENTS = 100;

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
		ServerManager	serverManager;

		parser.loadMimeTypes(types);
		for (size_t i = 0; i < config.servers.size(); ++i)
			serverManager.addServer(config.servers[i].listenPort, MAX_CLIENTS);

		serverManager.pollEvents();
	}
	catch (const std::exception& e)
	{
		return (std::cerr << "Error: " << e.what() << std::endl, EXIT_FAILURE);
	}
	return 0;
}
