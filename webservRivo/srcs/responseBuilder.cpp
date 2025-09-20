/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:28 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:25:41 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"

HttpResponseBuilder::HttpResponseBuilder(const MimeTypes &types)
        :  _mimeTypes(types) {}

HttpResponseBuilder::~HttpResponseBuilder() {}

std::string HttpResponseBuilder::getMimeType(const std::string &path)
{
    std::string::size_type dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return ("application/octet-stream");

    std::string ext = path.substr(dotPos + 1);
    std::map<std::string, std::string>::const_iterator it = _mimeTypes.types.find(ext);
    if (it != _mimeTypes.types.end())
        return (it->second);
    return ("application/octet-stream");
}

std::string HttpResponseBuilder::buildResponse(const HttpRequest &req, const ServerConfig_t &serverConf, const LocationConfig_t &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        if (req.uri.find(locationConf.cgiExtension) != std::string::npos && !locationConf.cgiPath.empty())
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            body = cgi.execute();
        }
        else
        {
            std::string filePath = serverConf.root + req.uri;

            if (req.uri == "/")
            {
                if (!serverConf.indexFiles.empty())
                    filePath = serverConf.root + "/" + serverConf.indexFiles[0];
                else
                    filePath = serverConf.root + "/index.html";
            }

            body = ftReadFile(filePath);

            std::string contentType = getMimeType(filePath);
            headers += "Content-Type: " + contentType + "\r\n";
        }

        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html\r\n";
    }

    return statusLine + headers + "\r\n" + body;
}
