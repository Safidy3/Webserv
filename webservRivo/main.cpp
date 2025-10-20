/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:40:36 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/07 13:45:25 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpConfig.hpp"
#include "../include/httpRequest.hpp"
#include "../include/httpServer.hpp"

#include <signal.h>

int main(int argc, char **argv)
{
    std::string configPath;
    std::string mimeTypesPath = "./conf.d/mime.type";

    if (argc == 2 && argv[1][0])
        configPath = argv[1];  
    else if (argc == 1)
        configPath = "./conf.d/webserv.conf";
    else {
        std::cerr << "Use: " << argv[0] << " [config_file]" << std::endl;
        return EXIT_FAILURE;
    }
    
    try
    {
        // Eviter que des EPIPE lors de send() fassent terminer le processus
        signal(SIGPIPE, SIG_IGN);
        ConfigParser parser(configPath, mimeTypesPath);

        HttpConfig config = parser.parse();

        MimeTypes types;
        parser.loadMimeTypes(types);

        Server server(config, types);
        server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
