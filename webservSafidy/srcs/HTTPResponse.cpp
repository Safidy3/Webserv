#include "../include/HTTPResponse.hpp"

void		HTTPResponse::setStatus(int code)
{
	if (code == 200)
		status = "200 OK";
	else if (code == 404)
		status = "404 Not Found";
	else if (code == 500)
		status = "500 Internal Server Error";
	else
		status = "400 Bad Request"; // default
}

void		HTTPResponse::setHeader(const std::string& key, const std::string& value)
{
	headers[key] = value;
}

void		HTTPResponse::setBody(const std::string& body)
{
	this->body = body;
}

std::string	HTTPResponse::ftToString()
{
	std::string response;
	response += "HTTP/1.1 " + status + "\r\n";
	for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
		response += header->first + ": " + header->second + "\r\n";
	response += "\r\n" + body;
	return response;
}