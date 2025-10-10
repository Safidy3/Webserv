#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "../webserv.hpp"

class ConfigParser
{
private:
	std::string content;
	size_t pos;

	// Helper functions
	void skipWhitespace();
	void skipComments();
	std::string getNextToken();
	std::string readUntil(char delimiter);
	std::string trim(const std::string &str);
	bool isWhitespace(char c);

	// Parsing functions
	void parseHttp(ServersConfig_t &config);
	void parseServer(ServerConfig_t &server);
	void parseLocation(LocationConfig_t &location);
	void parseServerDirective(const std::string &directive, ServerConfig_t &server);
	void parseLocationDirective(const std::string &directive, LocationConfig_t &location);

	// Value parsing
	std::vector<std::string> parseList(const std::string &value);
	void parseListenDirective(const std::string &value, ServerConfig_t &server);
	void parseErrorPage(const std::string &value, ServerConfig_t &server);

public:
	ConfigParser();
	ServersConfig_t parse(const std::string &filename);

	// Print methods
	static void printConfig(const ServersConfig_t &config);
	static void printServer(const ServerConfig_t &server, int serverNum);
	static void printLocation(const LocationConfig_t &location);
};

// Implementation

ConfigParser::ConfigParser() : pos(0) {}

bool ConfigParser::isWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void ConfigParser::skipWhitespace()
{
	while (pos < content.length() && isWhitespace(content[pos]))
	{
		pos++;
	}
}

void ConfigParser::skipComments()
{
	skipWhitespace();
	while (pos < content.length() && content[pos] == '#')
	{
		while (pos < content.length() && content[pos] != '\n')
		{
			pos++;
		}
		skipWhitespace();
	}
}

std::string ConfigParser::trim(const std::string &str)
{
	size_t start = 0;
	size_t end = str.length();

	while (start < end && isWhitespace(str[start]))
		start++;
	while (end > start && isWhitespace(str[end - 1]))
		end--;
	return str.substr(start, end - start);
}

std::string ConfigParser::getNextToken()
{
	skipComments();

	if (pos >= content.length())
		return "";

	std::string token;

	// Handle braces
	if (content[pos] == '{' || content[pos] == '}')
	{
		token += content[pos++];
		return token;
	}

	// Handle semicolons
	if (content[pos] == ';')
	{
		token += content[pos++];
		return token;
	}

	// Read regular token
	while (pos < content.length() && !isWhitespace(content[pos]) &&
		   content[pos] != '{' && content[pos] != '}' && content[pos] != ';')
		token += content[pos++];
	return token;
}

std::string ConfigParser::readUntil(char delimiter)
{
	std::string result;
	skipWhitespace();

	while (pos < content.length() && content[pos] != delimiter)
		result += content[pos++];
	return trim(result);
}

std::vector<std::string> ConfigParser::parseList(const std::string &value)
{
	std::vector<std::string> result;
	std::istringstream iss(value);
	std::string item;

	while (iss >> item)
		result.push_back(item);
	return result;
}

void ConfigParser::parseListenDirective(const std::string &value, ServerConfig_t &server)
{
	size_t colon_pos = value.find(':');

	if (colon_pos != std::string::npos)
	{
		server.listen_address = value.substr(0, colon_pos);
		std::string port_str = value.substr(colon_pos + 1);
		std::istringstream iss(port_str);
		iss >> server.listen_port;
	}
	else
	{
		std::istringstream iss(value);
		if (!(iss >> server.listen_port))
			server.listen_address = value;
	}
}

void ConfigParser::parseErrorPage(const std::string &value, ServerConfig_t &server)
{
	std::istringstream iss(value);
	int code;
	std::string path;

	if (iss >> code >> path)
		server.error_pages[code] = path;
}

void ConfigParser::parseServerDirective(const std::string &directive, ServerConfig_t &server)
{
	std::string value = readUntil(';');

	if (directive == "listen")
		parseListenDirective(value, server);
	else if (directive == "server_name")
		server.server_name = value;
	else if (directive == "root")
		server.root = (value[value.size() - 1] == '/') ? value : value + '/';
	else if (directive == "log_path")
		server.log_path = value;
	else if (directive == "index")
		server.index = parseList(value);
	else if (directive == "client_max_body_size")
	{
		std::istringstream iss(value);
		iss >> server.client_max_body_size;
	}
	else if (directive == "error_page")
		parseErrorPage(value, server);
	else if (directive == "keepalive_timeout")
	{
		std::istringstream iss(value);
		iss >> server.keepalive_timeout;
	}
	getNextToken(); // consume semicolon
}

void ConfigParser::parseLocationDirective(const std::string &directive, LocationConfig_t &location)
{
	std::string value = readUntil(';');

	if (directive == "root")
		location.root = value;
	else if (directive == "index")
		location.index = parseList(value);
	else if (directive == "methods")
		location.methods = parseList(value);
	else if (directive == "autoindex")
		location.autoindex = (value == "on");
	else if (directive == "return")
	{
		std::istringstream iss(value);
		iss >> location.redirect_code >> location.redirect_url;
	}
	else if (directive == "cgi_extension")
		location.cgi_extension = value;
	else if (directive == "cgi_path")
		location.cgi_path = value;
	getNextToken(); // consume semicolon
}

void ConfigParser::parseLocation(LocationConfig_t &location)
{
	location.path = getNextToken();

	std::string brace = getNextToken();
	if (brace != "{")
		throw std::runtime_error("Expected '{' after location path");

	std::string token;
	while ((token = getNextToken()) != "}")
	{
		if (token.empty())
			throw std::runtime_error("Unexpected end of file in location block");
		parseLocationDirective(token, location);
	}
}

void ConfigParser::parseServer(ServerConfig_t &server)
{
	std::string brace = getNextToken();
	if (brace != "{")
		throw std::runtime_error("Expected '{' after server directive");

	std::string token;
	while ((token = getNextToken()) != "}")
	{
		if (token.empty())
			throw std::runtime_error("Unexpected end of file in server block");
		if (token == "location")
		{
			LocationConfig_t location;
			parseLocation(location);
			server.locations.push_back(location);
		}
		else
			parseServerDirective(token, server);
	}
}

void ConfigParser::parseHttp(ServersConfig_t &config)
{
	std::string brace = getNextToken();
	if (brace != "{")
		throw std::runtime_error("Expected '{' after http directive");

	std::string token;
	while ((token = getNextToken()) != "}")
	{
		if (token.empty())
			throw std::runtime_error("Unexpected end of file in http block");
		if (token == "server")
		{
			ServerConfig_t server;
			parseServer(server);
			config.push_back(server);
		}
	}
}

ServersConfig_t ConfigParser::parse(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file: " + filename);

	std::stringstream buffer;
	buffer << file.rdbuf();
	content = buffer.str();
	pos = 0;

	ServersConfig_t config;

	std::string token = getNextToken();
	if (token == "http")
		parseHttp(config);
	else
		throw std::runtime_error("Expected 'http' directive at top level");
	return config;
}

// Print methods implementation
void ConfigParser::printLocation(const LocationConfig_t &location)
{
	std::cout << "      Location: " << location.path << std::endl;
	if (!location.root.empty())
	{
		std::cout << "        Root: " << location.root << std::endl;
	}
	if (!location.index.empty())
	{
		std::cout << "        Index: ";
		for (size_t i = 0; i < location.index.size(); i++)
		{
			std::cout << location.index[i];
			if (i < location.index.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	if (!location.methods.empty())
	{
		std::cout << "        Methods: ";
		for (size_t i = 0; i < location.methods.size(); i++)
		{
			std::cout << location.methods[i];
			if (i < location.methods.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "        Autoindex: " << (location.autoindex ? "on" : "off") << std::endl;
	if (location.redirect_code != 0)
	{
		std::cout << "        Redirect: " << location.redirect_code << " "
				  << location.redirect_url << std::endl;
	}
	if (!location.cgi_extension.empty())
	{
		std::cout << "        CGI Extension: " << location.cgi_extension << std::endl;
	}
	if (!location.cgi_path.empty())
	{
		std::cout << "        CGI Path: " << location.cgi_path << std::endl;
	}
}

void ConfigParser::printServer(const ServerConfig_t &server, int serverNum)
{
	std::cout << "\n  Server " << serverNum << ":" << std::endl;
	std::cout << "    Listen: " << server.listen_address << ":"
			  << server.listen_port << std::endl;
	if (!server.server_name.empty())
		std::cout << "    Server Name: " << server.server_name << std::endl;
	if (!server.root.empty())
		std::cout << "    Root: " << server.root << std::endl;
	if (!server.log_path.empty())
		std::cout << "    Log Path: " << server.log_path << std::endl;
	if (!server.index.empty())
	{
		std::cout << "    Index: ";
		for (size_t i = 0; i < server.index.size(); i++)
		{
			std::cout << server.index[i];
			if (i < server.index.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "    Client Max Body Size: " << server.client_max_body_size << std::endl;
	std::cout << "    Keepalive Timeout: " << server.keepalive_timeout << std::endl;

	if (!server.error_pages.empty())
	{
		std::cout << "    Error Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator it = server.error_pages.begin();
			 it != server.error_pages.end(); ++it)
			std::cout << "      " << it->first << " -> " << it->second << std::endl;
	}

	if (!server.locations.empty())
	{
		std::cout << "    Locations:" << std::endl;
		for (size_t i = 0; i < server.locations.size(); i++)
			printLocation(server.locations[i]);
	}
}

void ConfigParser::printConfig(const ServersConfig_t &config)
{
	std::cout << "========================================" << std::endl;
	std::cout << "Configuration Summary" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "Total Servers: " << config.size() << std::endl;

	for (size_t i = 0; i < config.size(); i++)
	{
		printServer(config[i], i + 1);
	}

	std::cout << "\n========================================" << std::endl;
}

#endif