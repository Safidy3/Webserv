#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "../webserv.hpp"

class HTTPRequest
{
private:
	std::string method;
	std::string path;
	std::string version;
	std::string body;
	std::map<std::string, std::string> headers;

public:
	HTTPRequest() : method(""), path(""), version(""), body(""), headers() {};
	HTTPRequest(const char *raw) { parseHttpRequest(raw); }
	~HTTPRequest(){};

	std::string getMethod() const { return method; }
	std::string getPath() const { return path; }
	std::string getVersion() const { return version; }
	std::string getBody() const { return body; }
	std::map<std::string, std::string> getHeaders() const { return headers; }

	bool	parseHttpRequest(const char *raw);
	void	printRequest();
	// bool	isComplete(); // (check if full request is received)
	// size_t	getContentLength();
};

#endif
