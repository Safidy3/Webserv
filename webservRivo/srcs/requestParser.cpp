#include "../include/httpRequest.hpp"

HttpRequestParser::HttpRequestParser() {}

HttpRequestParser::~HttpRequestParser() {}

HttpRequest HttpRequestParser::parseRequest(const std::string &rawRequest)
{
    HttpRequest request;

    std::istringstream stream(rawRequest);
    std::string line;

    // 1. Parse request line
    if (!std::getline(stream, line) || line.empty())
        throw std::runtime_error("Invalid HTTP request: missing request line");
    if (line[line.size() - 1] == '\r') line.erase(line.size() - 1); // remove \r
    parseRequestLine(line, request);

    // 2. Parse headers
    std::vector<std::string> headerLines;
    while (std::getline(stream, line) && line != "\r" && !line.empty())
    {
        if (line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        headerLines.push_back(line);
    }
    parseHeaders(headerLines, request);

    // 3. Parse body (everything after headers)
    std::string body;
    if (std::getline(stream, body, '\0'))
        parseBody(body, request);

    return request;
}

void HttpRequestParser::parseRequestLine(const std::string &requestLine, HttpRequest &request)
{
    std::istringstream iss(requestLine);
    if (!(iss >> request.method >> request.uri >> request.httpVersion))
        throw std::runtime_error("Malformed request line");

    parseUri(request.uri, request);
}

void HttpRequestParser::parseHeaders(const std::vector<std::string> &headerLines, HttpRequest &request)
{
    for (size_t i = 0; i < headerLines.size(); ++i)
    {
        size_t colonPos = headerLines[i].find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string key = headerLines[i].substr(0, colonPos);
        std::string value = headerLines[i].substr(colonPos + 1);

        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        request.headers[key] = value;

        if (key == "Host")
            parseHostHeader(value, request);
        else if (key == "Content-Length")
            request.contentLength = static_cast<size_t>(ftToInt(value.c_str()));
        else if (key == "Content-Type")
        {
            size_t pos = value.find("boundary=");
            if (pos != std::string::npos)
                request.boundary = value.substr(pos + 9);
        }
    }
}

void HttpRequestParser::parseBody(const std::string &body, HttpRequest &request)
{
    request.body = body;
}

void HttpRequestParser::parseUri(const std::string &uri, HttpRequest &request)
{
    size_t queryPos = uri.find('?');
    size_t fragmentPos = uri.find('#');

    request.queryString.clear();
    request.fragment.clear();

    if (queryPos != std::string::npos)
    {
        request.queryString = uri.substr(queryPos + 1, fragmentPos - queryPos - 1);
    }
    if (fragmentPos != std::string::npos)
    {
        request.fragment = uri.substr(fragmentPos + 1);
    }
}

void HttpRequestParser::parseHostHeader(const std::string &hostHeader, HttpRequest &request)
{
    size_t colonPos = hostHeader.find(':');
    if (colonPos != std::string::npos)
    {
        request.host = hostHeader.substr(0, colonPos);
        request.port = ftToInt(hostHeader.substr(colonPos + 1).c_str());
    }
    else
    {
        request.host = hostHeader;
        request.port = 80;
    }
}
