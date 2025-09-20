#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../webserv.hpp"

class HTTPResponse
{
private:
	std::string	status;
	std::string	body;
	std::map<std::string, std::string>	headers;

public:
	HTTPResponse(){};
	~HTTPResponse(){};

	void		setStatus(int code)
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

	void		setHeader(const std::string& key, const std::string& value)
	{
		headers[key] = value;
	}

	void		setBody(const std::string& body)
	{
		this->body = body;
	}

	std::string	getBody() const { return body; }
	std::string	getStatus() const { return status; }

	std::string	ftToString()
	{
		std::string response;
		response += "HTTP/1.1 " + status + "\r\n";
		for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
			response += header->first + ": " + header->second + "\r\n";
		response += "\r\n" + body;
		return response;
	}
};

#endif
