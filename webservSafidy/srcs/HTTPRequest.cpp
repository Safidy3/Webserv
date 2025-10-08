#include "../include/HTTPRequest.hpp"

void	HTTPRequest::parseHttpRequest(const char *raw)
{
	const std::string &rawRequest = std::string(raw);

	std::istringstream stream(rawRequest);
	std::string line;

	// 1. Parse request line
	if (!std::getline(stream, line) || line.empty())
	{
		isValidRequest = false;
		throw std::runtime_error("Invalid HTTP request: missing request line");
	}
	if (line[line.size() - 1] == '\r') line.erase(line.size() - 1); // remove \r
	parseRequestLine(line);

	// 2. Parse headers
	std::vector<std::string> headerLines;
	while (std::getline(stream, line) && line != "\r" && !line.empty())
	{
		if (line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		headerLines.push_back(line);
	}
	parseHeaders(headerLines);

	// 3. Parse body (everything after headers)
	std::string http_body;
	if (std::getline(stream, http_body, '\0'))
		body = http_body;
		// parseBody(http_body);
}

void HTTPRequest::parseRequestLine(const std::string &requestLine)
{
	std::istringstream iss(requestLine);
	if (!(iss >> method >> uri >> httpVersion))
	{
		isValidRequest = false;
		throw std::runtime_error("Malformed request line");
	}
	parseUri(uri);
}

void HTTPRequest::parseHeaders(const std::vector<std::string> &headerLines)
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

		headers[key] = value;

		if (key == "Host")
			parseHostHeader(value);
		else if (key == "Content-Length")
			contentLength = static_cast<size_t>(ftToInt(value.c_str()));
		else if (key == "Content-Type")
		{
			size_t pos = value.find("boundary=");
			if (pos != std::string::npos)
				boundary = value.substr(pos + 9);
		}
	}
}

// void HTTPRequest::parseBody(const std::string &body)
// {
// 	// body = body;
// }

void HTTPRequest::parseUri(const std::string &uri)
{
	size_t queryPos = uri.find('?');
	size_t fragmentPos = uri.find('#');

	queryString.clear();
	fragment.clear();

	if (queryPos != std::string::npos)
		queryString = uri.substr(queryPos + 1, fragmentPos - queryPos - 1);
	if (fragmentPos != std::string::npos)
		fragment = uri.substr(fragmentPos + 1);
}

void HTTPRequest::parseHostHeader(const std::string &hostHeader)
{
	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos)
	{
		host = hostHeader.substr(0, colonPos);
		port = ftToInt(hostHeader.substr(colonPos + 1).c_str());
	}
	else
	{
		host = hostHeader;
		port = 80;
	}
}

void    HTTPRequest::printRequest() const
{
	std::cout << "\nRequest details:\n";
	std::cout << "\tMethod: " << method << "\n";
	std::cout << "\tURI: " << uri << "\n";
	std::cout << "\tHTTP Version: " << httpVersion << "\n";
	std::cout << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << "\t" << it->first << ": " << it->second << "\n";
	
	std::cout << "\n\tHost: " << host << "\n";
	std::cout << "\tPort: " << port << "\n";
	std::cout << "\tContent-Length: " << contentLength << "\n";
	std::cout << "\tBoundary: " << boundary << "\n";
	std::cout << "\tBody:\n" << body << "\n";
	std::cout << "\tQuery String: " << queryString << "\n";
	std::cout << "\tFragment: " << fragment << "\n";
}