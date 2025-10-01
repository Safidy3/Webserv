#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "../webserv.hpp"

class HTTPRequest
{
private:
	// Request line
	std::string method;
	std::string uri;
	std::string queryString;
	std::string fragment;
	std::string httpVersion;

	// Headers
	std::map<std::string, std::string> headers;
	std::string host;
	int         port;

	// Body
	std::string body;
	std::string boundary;
	size_t      contentLength;

	std::string resolvedPath;

	void	parseRequestLine(const std::string &requestLine);
	void	parseHeaders(const std::vector<std::string> &headerLines);
	void	parseUri(const std::string &uri);
	void	parseHostHeader(const std::string &hostHeader);
	// void	parseBody(const std::string &body);

public:
	HTTPRequest() : method(""), uri(""), queryString(""), fragment(""), httpVersion(""),
					host(""), port(80), body(""), boundary(""), contentLength(0), resolvedPath("") {}
	HTTPRequest(const char *raw) { parseHttpRequest(raw); }
	~HTTPRequest(){};

	void	parseHttpRequest(const char *raw);
	void	printRequest() const;
	size_t	getContentLength();
};

#endif
