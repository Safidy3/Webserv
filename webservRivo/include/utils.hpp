/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 16:22:27 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/21 16:09:20 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "httpServer.hpp"

class ServerConfig;
class LocationConfig;
class HttpRequest;


#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <map>

class HttpRequest;
class ServerConfig;
class LocationConfig;

template <typename T>
std::string ftToString(T value)
{
    std::ostringstream ss;
    ss << value;
    return (ss.str());
}

int ftToInt(const std::string &s);
std::string ftStrdup(const char* s);
std::string ftReadFile(const std::string &path);
std::vector<std::string> ftSplitStr(const std::string& str, const std::string& delimiter);
std::vector<std::string> splitParts(const std::string &body, const std::string &delimiter);
std::string extractFilename(const std::string &part);
std::string extractFileContent(const std::string &part);
void appendToCSV(const std::map<std::string, std::string> &fields, const std::string &csvPath, const std::string &uploadedFileName);
std::string extractFieldName(const std::string &part);

std::string normalizeRelativePath(const std::string &relative);
bool isPathInsideRoot(const std::string &root, const std::string &target, std::string &outCanonicalTarget);
std::string generateAutoindexHTML(const std::string &dirPath, const std::string &uri);
std::string parseCGIStatusFromHeaders(const std::string &headers);
bool checkClientMaxBodySize(size_t contentLength, size_t clientMaxBodySize);
// Dechunk a Transfer-Encoding: chunked body. Returns empty string on parse error.
std::string dechunkBody(const std::string &chunkedBody);

bool isCgiRequest(const HttpRequest &req, const LocationConfig &locationConf, std::string &cgiPath);
std::string resolveFilePath(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf);

#endif
