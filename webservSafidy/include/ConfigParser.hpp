#ifndef CONF_PARSER_HPP
#define CONF_PARSER_HPP

#include "../webserv.hpp"

struct HttpConfig
{
	std::vector<ServerConfig_t> servers;
};

struct MimeTypes
{
	std::map<std::string, std::string> types;
};

class ConfigParser
{
	private:
		std::string _configFilePath;
		std::string _mimeTypesPath;
		std::string _fileContent;
		// MimeTypes &_mineTypes;

		void    expectToken(std::istream &input, const std::string &expected);
		void    parseHttpBlock(std::istream &input, HttpConfig &httpConfig);
		void    parseServerBlock(std::istream &input, ServerConfig_t &config);
		void    parseLocationBlock(std::istream &input, LocationConfig_t &loc);

	public:
		ConfigParser(const std::string &_configFilePath, const std::string &_mimeTypesPath);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser();

		void loadMimeTypes(MimeTypes &mimeTypes);
		HttpConfig  parse();
};

#endif
