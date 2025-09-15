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

// Utility function to trim whitespace
std::string	trim(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

// Utility function to convert to lowercase
std::string	toLower(const std::string &str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

bool	HTTPRequest::parseHttpRequest(const char *Request)
{
	if (Request == NULL || strlen(Request) == 0)
		return false;

	const std::string &rawRequest = std::string(Request);

	// Split into lines using \r\n
	std::vector<std::string>	lines;
	size_t						pos = 0;
	size_t						found = 0;

	while ((found = rawRequest.find("\r\n", pos)) != std::string::npos)
	{
		lines.push_back(rawRequest.substr(pos, found - pos));
		pos = found + 2;
	}
	// Handle case where last line doesn't end with \r\n
	if (pos < rawRequest.length())
		lines.push_back(rawRequest.substr(pos));

	if (lines.empty())
		return false;

	// Parse request line (first line): METHOD PATH VERSION
	std::istringstream requestLine(lines[0]);
	if (!(requestLine >> method >> path >> version))
		return false;

	// Convert method to uppercase for consistency
	std::transform(method.begin(), method.end(), method.begin(), ::toupper);

	// Parse headers
	size_t headerEnd = 1;
	for (size_t i = 1; i < lines.size(); ++i)
	{
		if (lines[i].empty())
		{
			// Empty line indicates end of headers
			headerEnd = i + 1;
			break;
		}

		// Find the colon separator
		size_t colonPos = lines[i].find(':');
		// Malformed header, skip it
		if (colonPos == std::string::npos)
			continue;

		std::string headerName = trim(lines[i].substr(0, colonPos));
		std::string headerValue = trim(lines[i].substr(colonPos + 1));

		// Store header name in lowercase for case-insensitive lookup
		headers[toLower(headerName)] = headerValue;
	}

	// Parse body (everything after the empty line)
	if (headerEnd < lines.size())
	{
		std::ostringstream bodyStream;
		for (size_t i = headerEnd; i < lines.size(); ++i)
		{
			if (i > headerEnd)
				bodyStream << "\r\n";
			bodyStream << lines[i];
		}
		body = bodyStream.str();
	}

	return true;
}

// Example usage function
void	HTTPRequest::printRequest()
{
	std::cout << "Method: " << method << std::endl;
	std::cout << "Path: " << path << std::endl;
	std::cout << "Version: " << version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	if (!body.empty())
		std::cout << "Body: " << body << std::endl;
}


#endif
