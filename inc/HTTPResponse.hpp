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
	HTTPResponse();
	~HTTPResponse();

	void		setStatus(int code);
	void		setHeader(const std::string&, const std::string&);
	void		setBody(const std::string&);
	std::string	toString(); // formats full HTTP response.
};

#endif
