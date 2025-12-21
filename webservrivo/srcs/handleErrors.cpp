/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleErrors.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 19:27:14 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/22 19:28:48 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/handleErrors.hpp"
#include <sys/socket.h>
#include <unistd.h>

std::map<int, std::string> HandleErrors::initReasonMap() {
    std::map<int, std::string> reasons;
    reasons[400] = "Bad Request";
    reasons[403] = "Forbidden";
    reasons[404] = "Not Found";
    reasons[405] = "Method Not Allowed";
    reasons[408] = "Request Timeout";
    reasons[413] = "Payload Too Large";
    reasons[500] = "Internal Server Error";
    reasons[505] = "HTTP Version Not Supported";
    return reasons;
}

std::string HandleErrors::getDefaultReason(int code) {
    static std::map<int, std::string> reasons = initReasonMap();
    if (reasons.find(code) != reasons.end())
        return reasons[code];
    return "Unknown Error";
}

std::string HandleErrors::getErrorBodyFromFile(const std::string &filePath, int code, const std::string &reason) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {

        std::ostringstream oss;
        oss << "<html><head><title>" << code << " " << reason 
            << "</title></head><body><h1>" 
            << code << " " << reason 
            << "</h1></body></html>";
        return oss.str();
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string HandleErrors::generateErrorResponse(int code, const ServerConfig& serverConf, const LocationConfig* locationConf, const std::string& extraHeaders)
{
    (void)locationConf;

    static std::map<int, std::string> reasons = initReasonMap();
    std::string reason = "Unknown Error";
    if (reasons.find(code) != reasons.end())
        reason = reasons[code];

    std::map<int,std::string>::const_iterator it = serverConf.errorPages.find(code);
    std::string body;

    if (it != serverConf.errorPages.end()) {
        std::string errorPath = it->second;

        if (!errorPath.empty() && errorPath[0] == '/')
            errorPath.erase(0, 1);

        std::string fullPath = serverConf.root + "/" + errorPath;

        std::ifstream file(fullPath.c_str());
        if (file) {
            std::ostringstream ss;
            ss << file.rdbuf();
            body = ss.str();
        }
    }

    if (body.empty()) {
        std::ostringstream ss;
        ss << "<html><head><title>" << code << " " << reason 
           << "</title></head><body><h1>" << code << " " << reason
           << "</h1></body></html>";
        body = ss.str();
    }

    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " " << reason << "\r\n";
    oss << "Content-Type: text/html\r\n";
    if (!extraHeaders.empty())
        oss << extraHeaders;
    oss << "Content-Length: " << body.size() << "\r\n\r\n";
    oss << body;

    return oss.str();
}
