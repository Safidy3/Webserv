/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main copy 2.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:43:48 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/11 08:53:50 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./include/httpConfig.hpp"

int main(int argc, char **argv)
{
    std::string file;

    if (argc == 2 && argv[1][0])
    {
        file = argv[1];  
    }
    else if (argc == 1)
    {
        file = "./conf.d/webserv.conf";
    }
    else
    {
        std::cerr << "Use: " << argv[0] << " [config_file]" << std::endl;
        return EXIT_FAILURE;
    }
    
    try
    {
        ConfigParser parser(file);
        HttpConfig config = parser.parse();

        std::cout << "Parsed " << config.servers.size() << " server block(s)." << std::endl;
        for (size_t i = 0; i < config.servers.size(); ++i)
        {
            std::cout << "Server " << i << " listens on port " << config.servers[i].listenPort << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Config error: " << e.what() << std::endl;
    }
}