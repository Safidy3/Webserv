/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:28 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/24 16:36:57 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"

HttpResponseBuilder::HttpResponseBuilder(const MimeTypes &types)
        : _mimeTypes(types) {}

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

std::string HttpResponseBuilder::buildResponse(
    const HttpRequest &req, 
    const ServerConfig &serverConf, 
    const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        // 🔹 Vérifier si la requête correspond à un CGI
        if (!locationConf.cgiExtension.empty() && req.uri.size() >= locationConf.cgiExtension.size() &&
            req.uri.compare(req.uri.size() - locationConf.cgiExtension.size(), locationConf.cgiExtension.size(),
            locationConf.cgiExtension) == 0 && !locationConf.cgiPath.empty())
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            std::string cgiOutput = cgi.execute();

            // Séparer headers et body envoyés par le CGI
            size_t pos = cgiOutput.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headers = cgiOutput.substr(0, pos);
                body = cgiOutput.substr(pos + 4);
            } else {
                headers = "Content-Type: text/html\r\n";
                body = cgiOutput;
            }

            if (headers.find("Content-Length") == std::string::npos)
                headers += "\r\nContent-Length: " + ftToString(body.size());
        }
        else
        {
            // 🔹 Fichier statique : gérer root location + index
            std::string root = locationConf.root.empty() ? serverConf.root : locationConf.root;

            // Supprimer le préfixe location de l'URI
            std::string relativePath;

            if (!locationConf.root.empty()) {
                // Si root est déjà le dossier exact de bootstrap, ne pas enlever /bootstrap/
                relativePath = req.uri.substr(1); // enlever juste le /
            } else {
                relativePath = req.uri.substr(locationConf.path.size());
            }

            // Si la requête pointe vers la racine de la location
            if (relativePath.empty() || relativePath == "/") {
                if (!locationConf.indexFiles.empty())
                    relativePath = locationConf.indexFiles[0];
                else if (!serverConf.indexFiles.empty())
                    relativePath = serverConf.indexFiles[0];
                else
                    relativePath = "index.html";
            }

            std::string filePath = root + "/" + relativePath;

            body = ftReadFile(filePath);
            std::string contentType = getMimeType(filePath);
            headers = "Content-Type: " + contentType + "; charset=UTF-8\r\n";
            headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
        }
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html; charset=UTF-8\r\n";
    }

    return statusLine + headers + "\r\n\r\n" + body;
}


