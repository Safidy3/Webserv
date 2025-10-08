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
	int			port;

	// Body
	std::string body;
	std::string boundary;
	size_t		contentLength;

	std::string resolvedPath;

	bool		isValidRequest;

	void	parseRequestLine(const std::string &requestLine);
	void	parseHeaders(const std::vector<std::string> &headerLines);
	void	parseUri(const std::string &uri);
	void	parseHostHeader(const std::string &hostHeader);
	// void	parseBody(const std::string &body);

public:
	HTTPRequest() : method(""), uri(""), queryString(""), fragment(""),
					httpVersion(""), host(""), port(80), body(""),
					boundary(""), contentLength(0), resolvedPath("") {}
	HTTPRequest(const char *raw) { parseHttpRequest(raw); }
	~HTTPRequest(){};

	void	parseHttpRequest(const char *raw);
	void	printRequest() const;

	const std::string&	getMethod() const { return method; }
	const std::string&	getUri() const { return uri; }
	const std::string&	getQueryString() const { return queryString; }
	const std::string&	getFragment() const { return fragment; }
	const std::string&	getHttpVersion() const { return httpVersion; }
	const std::string&	getHost() const { return host; }
	int					getPort() const { return port; }
	const std::string&	getBody() const { return body; }
	const std::string&	getBoundary() const { return boundary; }
	size_t				getContentLength() const { return contentLength; }
	const std::map<std::string, std::string>& getHeaders() const { return headers; }
};

#endif
