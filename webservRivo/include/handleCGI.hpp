/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:47:11 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/16 14:15:02 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLE_CGI_HPP
#define HANDLE_CGI_HPP

#include "httpRequest.hpp"
#include "httpConfig.hpp"
#include "utils.hpp"
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

class HandleCGI
{
    private:
        const HttpRequest& _request;
        const ServerConfig& _serverConf;
        const LocationConfig& _locationConf;

        std::map<std::string, std::string> _env;

    public:
        HandleCGI(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &_locationConf);
        ~HandleCGI();

        void printEnv() const;
        void buildEnv();
        std::vector<char*> buildEnvArray() const;
        std::string execute();
};

#endif
