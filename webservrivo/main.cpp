/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:40:36 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 13:18:05 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpConfig.hpp"
#include "../include/httpRequest.hpp"
#include "../include/httpServer.hpp"

#include <signal.h>

volatile sig_atomic_t g_stop = 0;

void handle_sigint(int)
{
    g_stop = 1;
}

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
        // Ignorer SIGPIPE pour éviter les crash lors de send()
        // Ignorer SIGCHLD - on va utiliser waitpid(WNOHANG) au lieu de handler
        signal(SIGPIPE, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGINT, handle_sigint);

        ConfigParser parser(configPath, mimeTypesPath);
        HttpConfig config = parser.parse();

        MimeTypes types;
        parser.loadMimeTypes(types);

        Server server(config, types);

        server.run();

        std::cout << "\nSIGINT received, server cleanup...\n";
        server.cleanup();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
