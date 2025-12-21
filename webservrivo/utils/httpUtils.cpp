/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:03:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:04:35 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <vector>

std::string normalizeRelativePath(const std::string &relative)
{
    std::vector<std::string> parts;
    std::istringstream ss(relative);
    std::string token;
    while (std::getline(ss, token, '/')) {
        if (token.empty() || token == ".") continue;
        if (token == "..") {
            if (!parts.empty()) parts.pop_back();
        } else parts.push_back(token);
    }
    std::string norm;
    for (size_t i = 0; i < parts.size(); ++i) {
        norm += "/" + parts[i];
    }
    if (norm.empty()) norm = "/";
    return norm;
}

bool isPathInsideRoot(const std::string &root, const std::string &target, std::string &outCanonicalTarget)
{
    char rootBuf[PATH_MAX];
    if (!realpath(root.c_str(), rootBuf)) return false;
    std::string canonRoot(rootBuf);

    char targetBuf[PATH_MAX];
    if (realpath(target.c_str(), targetBuf)) {
        outCanonicalTarget = std::string(targetBuf);
    } else {
        outCanonicalTarget = canonRoot + normalizeRelativePath(target);
    }
    return outCanonicalTarget.find(canonRoot) == 0;
}

bool checkClientMaxBodySize(size_t contentLength, size_t clientMaxBodySize)
{
    if (clientMaxBodySize == 0) return false;
    return contentLength > clientMaxBodySize;
}

std::string dechunkBody(const std::string &chunkedBody)
{
    std::string out;
    size_t pos = 0;
    while (pos < chunkedBody.size()) {
        size_t lineEnd = chunkedBody.find("\r\n", pos);
        if (lineEnd == std::string::npos) return std::string();
        std::string sizeLine = chunkedBody.substr(pos, lineEnd - pos);
        size_t semi = sizeLine.find(';');
        std::string sizeHex = (semi == std::string::npos) ? sizeLine : sizeLine.substr(0, semi);
        unsigned long chunkSize = strtoul(sizeHex.c_str(), NULL, 16);
        pos = lineEnd + 2;
        
        if (chunkSize == 0) {
            size_t endPos = chunkedBody.find("\r\n\r\n", pos);
            if (endPos == std::string::npos) {

                if (pos + 2 <= chunkedBody.size()) return out;
                return std::string();
            }
            return out;
        }
        
        if (pos + chunkSize > chunkedBody.size()) return std::string();
        out.append(chunkedBody.data() + pos, chunkSize);
        pos += chunkSize;

        if (pos + 2 > chunkedBody.size()) return std::string();
        if (chunkedBody[pos] != '\r' || chunkedBody[pos+1] != '\n') return std::string();
        pos += 2;
    }
    return std::string();
}

bool isCgiRequest(const HttpRequest &req, const LocationConfig &locationConf, std::string &cgiPath)
{
    if (locationConf.cgiExtension.empty())
        return false;
    if (req.uri.size() < locationConf.cgiExtension.size())
        return false;

    if (req.uri.compare(req.uri.size() - locationConf.cgiExtension.size(),
        locationConf.cgiExtension.size(), locationConf.cgiExtension) == 0)
    {
        cgiPath = locationConf.root + req.uri.substr(locationConf.path.size());
        return true;
    }
    return false;
}

std::string resolveFilePath(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf)
{
    std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;
    std::string uri = req.uri;

    std::string relativePath;
    if (!locationConf.path.empty() && uri.find(locationConf.path) == 0)
        relativePath = uri.substr(locationConf.path.size());
    else
        relativePath = uri;

    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);

    std::string filePath = root + "/" + relativePath;

    while (filePath.find("//") != std::string::npos)
        filePath.replace(filePath.find("//"), 2, "/");

    return filePath;
}