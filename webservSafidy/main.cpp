/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/11 14:39:48 by safandri          #+#    #+#             */
/*   Updated: 2025/10/17 15:10:10 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "include/ConfigParser.hpp"
#include "include/ServerManager.hpp"
// #include "include/Server.hpp"
// #include "include/Client.hpp"
// #include "include/HTTPRequest.hpp"
// #include "include/HTTPResponse.hpp"

MimeTypes loadMimeTypes(const std::string& path)
{
	MimeTypes mimeTypes;
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open MIME types file: " + path);

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
			continue; // Skip empty lines and comments

		std::istringstream iss(line);
		std::string type;
		if (!(iss >> type))
			continue; // Skip malformed lines

		std::string extension;
		while (iss >> extension)
			mimeTypes[extension] = type;
	}
	return mimeTypes;
}

/*
*/
int main(int argc, char **argv)
{
	std::string configPath;
	std::string mimeTypesPath = "./conf.d/mime.type";

	if (argc == 2 && argv[1][0])
		configPath = argv[1];
	else if (argc == 1)
		configPath = "./conf.d/test2.conf";
	else
		return (std::cerr << "Use: " << argv[0] << " [config_file]" << std::endl, EXIT_FAILURE);
	try
	{
		MimeTypes		mimes;
		ConfigParser	parser;
		ServersConfig_t	config;
		ServerManager	pollManager;

		config = parser.parse(configPath);
		mimes = loadMimeTypes(mimeTypesPath);

		// Access parsed data
		for (size_t i = 0; i < config.size(); i++)
		{
			pollManager.addServer(config[i], mimes);
			std::cout << "\n=========================================\n\n";
		}

		pollManager.pollEvents();

	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}


// int main()
// {
//     std::string rawRequest = 
// 		// "POST /cgi_bin/form_handler.py HTTP/1.1\r\n"
//         "POST /test/demo_form.php?getname1=value1&getname2=value2 HTTP/1.1\r\n"
//         "Host: localhost:8080\r\n"
//         "Connection: keep-alive\r\n"
//         "Content-Length: 16\r\n"
//         "sec-ch-ua-platform: \"Linux\"\r\n"
//         "Origin: http://localhost:8080\r\n"
//         "Content-Type: application/x-www-form-urlencoded\r\n"
//         "Referer: http://localhost:8080/html/cgi_test.html\r\n"
//         "\r\n"
//         "postname1=value1&postname2=value2";
	
		
// 		HTTPRequestParser	parser;
// 		HTTPRequest			request;

// 		parser.parse(rawRequest);
// 		request = parser.getRequest();
		
// 		request.printRequest();

//     return 0;
// }