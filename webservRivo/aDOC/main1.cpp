/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main1.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:43:48 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/11 17:12:45 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./include/httpConfig.hpp"
#include "./include/httpRequest.hpp"

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

    }
    catch (const std::exception& e)
    {
        std::cerr << "Config error: " << e.what() << std::endl;
    }
}