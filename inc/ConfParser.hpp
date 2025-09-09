#ifndef CONF_PARSER_HPP
#define CONF_PARSER_HPP

#include "../webserv.hpp"

class ConfParser
{
public:
	ConfParser();
	~ConfParser();

	void parseConfig(const std::string &configFile);
};

#endif
