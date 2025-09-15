#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "../webserv.hpp"

class ErrorHandler
{
public:
	ErrorHandler();
	~ErrorHandler();

	std::string generateErrorPage(int code, const ServerConfig_t&);
};

#endif
