/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:36 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:25:41 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "handleCGI.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

class HttpResponseBuilder
{
    private:
        const MimeTypes &_mimeTypes;

    public:
        HttpResponseBuilder(const MimeTypes &types);
        ~HttpResponseBuilder();


        std::string getMimeType(const std::string &path);
        std::string buildResponse(const HttpRequest &req, const ServerConfig_t &serverConf, const LocationConfig_t &locationConf);
};

#endif
