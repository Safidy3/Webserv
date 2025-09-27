/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:43:48 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/16 16:40:06 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./include/httpConfig.hpp"

int main(int argc, char **argv)
{
    std::string configFile = "./conf.d/webserv.conf";
    std::string mimeTypesPath = "./conf.d/mime.type";
    if (argc == 2)
        configFile = argv[1];

    try
    {
        ConfigParser parser(configFile, mimeTypesPath);
        HttpConfig httpConfig = parser.parse();

        std::cout << "HTTP Config:" << std::endl;
        for (size_t i = 0; i < httpConfig.servers.size(); ++i)
        {
            const ServerConfig &server = httpConfig.servers[i];
            std::cout << "Server " << i + 1 << ":" << std::endl;
            std::cout << "  Host: " << server.host << std::endl;
            std::cout << "  Listen Port: " << server.listenPort << std::endl;
            std::cout << "  Root: " << server.root << std::endl;
            std::cout << "  Client Max Body Size: " << server.clientMaxBodySize << std::endl;

            std::cout << "  Index Files: ";
            for (size_t j = 0; j < server.indexFiles.size(); ++j)
                std::cout << server.indexFiles[j] << " ";
            std::cout << std::endl;

            std::cout << "  Error Pages:" << std::endl;
            for (std::map<int,std::string>::const_iterator it = server.errorPages.begin(); it != server.errorPages.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << std::endl;

            for (size_t j = 0; j < server.locations.size(); ++j)
            {
                const LocationConfig &loc = server.locations[j];
                std::cout << "  Location " << j + 1 << ": " << loc.path << std::endl;
                std::cout << "    Root: " << loc.root << std::endl;

                std::cout << "    Index Files: ";
                for (size_t k = 0; k < loc.indexFiles.size(); ++k)
                    std::cout << loc.indexFiles[k] << " ";
                std::cout << std::endl;

                std::cout << "    Methods: ";
                for (size_t k = 0; k < loc.methods.size(); ++k)
                    std::cout << loc.methods[k] << " ";
                std::cout << std::endl;

                std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
                if (loc.returnCode != 0)
                    std::cout << "    Return: " << loc.returnCode << " " << loc.returnPath << std::endl;
                if (!loc.cgiExtension.empty())
                    std::cout << "    CGI: " << loc.cgiExtension << " -> " << loc.cgiPath << std::endl;
                if (!loc.uploadDir.empty())
                    std::cout << "    Upload Dir: " << loc.uploadDir << std::endl;
                if (!loc.defaultFile.empty())
                    std::cout << "    Default File: " << loc.defaultFile << std::endl;

                if (!loc.directives.empty())
                {
                    std::cout << "    Unknown directives:" << std::endl;
                    for (std::map<std::string,std::string>::const_iterator it = loc.directives.begin(); it != loc.directives.end(); ++it)
                        std::cout << "      " << it->first << " -> " << it->second << std::endl;
                }
            }

            std::cout << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Config error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
