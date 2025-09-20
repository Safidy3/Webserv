/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:47:11 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:25:41 by safandri         ###   ########.fr       */
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
        const ServerConfig_t& _serverConf;
        const LocationConfig_t& _locationConf;

        std::map<std::string, std::string> _env;

    public:
        HandleCGI(const HttpRequest &req, const ServerConfig_t &serverConf, const LocationConfig_t &_locationConf);
        ~HandleCGI();

        void buildEnv();
        std::vector<char*> buildEnvArray() const;
        std::string execute();
};

#endif
