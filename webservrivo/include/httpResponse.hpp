/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:36 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 13:09:05 by rhanitra         ###   ########.fr       */
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

        static std::string parseCGIStatusFromHeaders(const std::string &headers);
        static std::string generateAutoindexHTML(const std::string &dirPath, const std::string &uri);
        static std::string reasonRedirect(int code);
        
        std::string buildResponse(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf);
};

#endif
