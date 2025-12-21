/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:28 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:17:19 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"
#include "../include/utils.hpp"
#include "../include/handleErrors.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <iostream>

HttpResponseBuilder::HttpResponseBuilder(const MimeTypes &types)
        : _mimeTypes(types) {}

HttpResponseBuilder::~HttpResponseBuilder() {}

std::string HttpResponseBuilder::getMimeType(const std::string &path)
{
    std::string::size_type dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream";

    std::string ext = path.substr(dotPos + 1);
    for (size_t i = 0; i < ext.size(); ++i) ext[i] = tolower(ext[i]);

    std::map<std::string, std::string>::const_iterator it = _mimeTypes.types.find(ext);
    if (it != _mimeTypes.types.end())
        return it->second;

    return "application/octet-stream";
}

std::string HttpResponseBuilder::parseCGIStatusFromHeaders(const std::string &headers)
{
    std::istringstream hh(headers);
    std::string line;
    while (std::getline(hh, line)) {
        if (!line.empty() && line[line.size()-1] == '\r') line.erase(line.size()-1);
        if (line.size() >= 7 && line.substr(0,7) == "Status:") {
            std::string status = line.substr(7);
            size_t s = status.find_first_not_of(" \t");
            if (s != std::string::npos) status = status.substr(s);
            return std::string("HTTP/1.0 ") + status + "\r\n";
        }
    }
    return std::string();
}

std::string HttpResponseBuilder::generateAutoindexHTML(const std::string &dirPath, const std::string &uri)
{
    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return std::string();
    std::ostringstream oss;
    oss << "<html><head><title>Index of " << uri << "</title></head><body>";
    oss << "<h1>Index of " << uri << "</h1><ul>";
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;
        oss << "<li><a href=\"" << name << "\">" << name << "</a></li>";
    }
    oss << "</ul></body></html>";
    closedir(dir);
    return oss.str();
}

std::string HttpResponseBuilder::reasonRedirect(int code)
{
    switch (code)
    {
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        default:  return "Redirect";
    }
}


std::string HttpResponseBuilder::buildResponse(
    const HttpRequest &req,
    const ServerConfig &serverConf,
    const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.0 200 OK\r\n";
    std::string headers;

    try
    {

        if (req.method == "DELETE") {
            std::string filePath = resolveFilePath(req, serverConf, locationConf);
            struct stat st;
            if (stat(filePath.c_str(), &st) != 0)
                return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);

            if (S_ISDIR(st.st_mode))
                return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);

            int writeAccess = access(filePath.c_str(), W_OK);
            std::cerr << "DELETE: checking access for " << filePath << " = " << writeAccess << std::endl;
            if (writeAccess != 0)
                return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);

            if (unlink(filePath.c_str()) != 0)
                return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);

            std::string response;
            response  = "HTTP/1.0 204 No Content\r\n";
            response += "Content-Length: 0\r\n";
            response += "Connection: close\r\n\r\n";
            return response;
        }


        if (!locationConf.returnPath.empty() && locationConf.returnCode >= 300 && locationConf.returnCode < 400)
        {
            std::ostringstream resp;

            resp << "HTTP/1.1 " << locationConf.returnCode << " " << reasonRedirect(locationConf.returnCode) << "\r\n";
            resp << "Location: " << locationConf.returnPath << "\r\n";
            resp << "Content-Length: 0\r\n";
            resp << "Connection: close\r\n\r\n";

            return resp.str();
        }

        std::string cgiScript;
        if (isCgiRequest(req, locationConf, cgiScript))
        {
            return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);
        }
        else
        {
            std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;

            std::string relativePath;
            if (!locationConf.path.empty() && req.uri.find(locationConf.path) == 0)
                relativePath = req.uri.substr(locationConf.path.size());
            else if (!req.uri.empty() && req.uri[0] == '/')
                relativePath = req.uri.substr(1);
            else
                relativePath = req.uri;

            if (relativePath.empty() || relativePath == "/") {
                if (!locationConf.indexFiles.empty())
                    relativePath = locationConf.indexFiles[0];
                else if (!serverConf.indexFiles.empty())
                    relativePath = serverConf.indexFiles[0];
            }

            std::string filePath = root + "/" + relativePath;
            struct stat st;
            if (stat(filePath.c_str(), &st) != 0) {
                std::string uriPath = req.uri;
                if (!uriPath.empty() && uriPath[0] == '/') uriPath = uriPath.substr(1);
                std::string altPath = root + "/" + uriPath;
                if (stat(altPath.c_str(), &st) == 0)
                    filePath = altPath;
            }

            if (file_exists(filePath))
            {
                if (S_ISDIR(st.st_mode))
                {
                    bool indexFound = false;
                    
                    for (size_t i = 0; i < locationConf.indexFiles.size(); ++i)
                    {
                        std::string idxPath = filePath + "/" + locationConf.indexFiles[i];
                        if (file_exists(idxPath))
                        {
                            body = ftReadFile(idxPath);
                            std::string contentType = getMimeType(idxPath);
                            if (contentType.find("text/") == 0 || contentType == "application/json")
                                contentType += "; charset=UTF-8";
                       
                            headers = "Content-Type: " + contentType + "\r\n";
                            headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                            indexFound = true;
                            break;
                        }
                    }
    
                    if (!indexFound)
                    {
                        for (size_t i = 0; i < serverConf.indexFiles.size(); ++i)
                        {
                            std::string idxPath = filePath + "/" + serverConf.indexFiles[i];
                            if (file_exists(idxPath))
                            {
                                body = ftReadFile(idxPath);
                                std::string contentType = getMimeType(idxPath);
                                if (contentType.find("text/") == 0 || contentType == "application/json")
                                    contentType += "; charset=UTF-8";
    
                                headers = "Content-Type: " + contentType + "\r\n";
                                headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                                indexFound = true;
                                break;
                            }
                        }
                    }
    
                    if (!indexFound)
                    {
                        if (locationConf.autoindex)
                        {
                            body = generateAutoindexHTML(filePath, req.uri);
                            headers = "Content-Type: text/html; charset=UTF-8\r\n";
                            headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                        }
                        else
                        {
                            return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);
                        }
                    }
                }
                
                else if (S_ISREG(st.st_mode))
                {

                    if (access(filePath.c_str(), R_OK) != 0) {
                        return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);
                    }
                    
                    body = ftReadFile(filePath);
                    std::string contentType = getMimeType(filePath);
                    if (contentType.find("text/") == 0 || contentType == "application/json")
                        contentType += "; charset=UTF-8";
    
                    headers = "Content-Type: " + contentType + "\r\n";
                    headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                }
                else
                    return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);
            }
            else
                return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);
        }
    }
    catch (...)
    {
        return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);
    }

    headers += "Cache-Control: no-store, no-cache, must-revalidate\r\n";
    headers += "Pragma: no-cache\r\n";
    headers += "Expires: 0\r\n";

    std::ostringstream response;
    if (headers.size() < 2 || headers.substr(headers.size() - 2) != "\r\n")
        headers += "\r\n";

    response << statusLine << headers << "\r\n";
    response.write(body.data(), body.size());
    return response.str();
}
