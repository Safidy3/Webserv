#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"

class Router
{
public:
	Router();
	~Router();

	std::string	resolvePath(const HTTPRequest&, const ServerConfig&);
	bool		methodAllowed(const HTTPRequest&, const ServerConfig&);
	bool		isCGI(const HTTPRequest&, const ServerConfig&);
};

#endif
