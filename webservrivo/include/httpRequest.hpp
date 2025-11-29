/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:12:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/11 17:12:21 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

struct HttpRequest
{
    // Request line
    std::string method;
    std::string uri;
    std::string queryString;
    std::string fragment;
    std::string httpVersion;

    // Headers
    std::map<std::string, std::string> headers;
    std::string host;
    int port;

    // Body
    std::string body;
    size_t contentLength;
    std::string boundary;

    std::string resolvedPath;

    HttpRequest() : port(80), contentLength(0) {}
};

class HttpRequestParser
{
    private:
        void parseRequestLine(const std::string &requestLine, HttpRequest &request);
        void parseHeaders(const std::vector<std::string> &headerLines, HttpRequest &request);
        void parseBody(const std::string &body, HttpRequest &request);
        void parseUri(const std::string &uri, HttpRequest &request);
        void parseHostHeader(const std::string &hostHeader, HttpRequest &request);

    public:
        HttpRequestParser();
        ~HttpRequestParser();

        HttpRequest parseRequest(const std::string &rawRequest);
};

#endif
