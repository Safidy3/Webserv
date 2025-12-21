/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:10:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/19 13:11:31 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpConfig.hpp"
#include "../include/configError.hpp"
#include <arpa/inet.h>

ConfigParser::ConfigParser(const std::string &configFilePathPath, const std::string &mimeTypesPath) 
    : _configFilePath(configFilePathPath), _mimeTypesPath(mimeTypesPath)
{
    _fileContent = ftReadFile(_configFilePath);
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
    *this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this != &other)
    {
        _configFilePath = other._configFilePath;
        _fileContent = other._fileContent;
    }
    return (*this);
}

ConfigParser::~ConfigParser() {}

void ConfigParser::loadMimeTypes(MimeTypes &mimeTypes)
{
    std::ifstream ifs(_mimeTypesPath.c_str());
    if (!ifs)
        throw std::runtime_error("cannot open mime.types");

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty() || line[0] == '#') continue;
        if (line.find("{") != std::string::npos || line.find("}") != std::string::npos) continue;

        std::istringstream iss(line);
        std::string mime;
        iss >> mime;
        std::string ext;
        while (iss >> ext)
        {
            if (!ext.empty() && ext[ext.size() - 1] == ';')
                ext.erase(ext.size() - 1);
            mimeTypes.types[ext] = mime;
        }
    }
}


void ConfigParser::expectToken(std::istream &input, const std::string &expected)
{
    std::string tok;
    input >> std::ws >> tok;
    if (tok != expected)
    {
        throw std::runtime_error("Expected \"" + expected + "\", got \"" + tok + "\"");
    }
}

void ConfigParser::parseLocationBlock(std::istream &input, LocationConfig &loc)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

        std::string value;
        std::getline(input, value, ';');

        size_t start = value.find_first_not_of(" \t");
        size_t end   = value.find_last_not_of(" \t");
        if (start != std::string::npos) value = value.substr(start, end - start + 1);

        if (token == "methods")
        {
            std::stringstream ss(value);
            std::string method;
            while (ss >> method) loc.methods.push_back(method);
            for (size_t i = 0; i < loc.methods.size(); ++i)
                if (loc.methods[i] != "GET" && loc.methods[i] != "POST" && loc.methods[i] != "DELETE")
                    throw ConfigError("Invalid method in location block: " + loc.methods[i]);
        }
        else if (token == "autoindex")
            loc.autoindex = (value == "on");
        else if (token == "index")
        {
            std::stringstream ss(value);
            std::string file;
            while (ss >> file) loc.indexFiles.push_back(file);
        }
        else if (token == "root") loc.root = value;
        else if (token == "upload_dir") loc.uploadDir = value;
        else if (token == "return")
        {
            std::stringstream ss(value);
            ss >> loc.returnCode;
            if (ss >> loc.returnPath) {}
        }
        else if (token == "redirect")
        {
            std::stringstream ss(value);
            std::string path;
            int code = 0;

            ss >> path;
            ss >> code;

            if (code == 0)
                code = 302;

            if (code < 300 || code >= 400)
                throw ConfigError("redirect: invalid status code");

            if (!isAbsoluteURL(path) && path[0] != '/')
                throw ConfigError("redirect: invalid URL (must be absolute or start with /)");

            loc.returnPath = path;
            loc.returnCode = code;
        }
        else if (token == "default_file") loc.defaultFile = value;
        else if (token == "cgi_extension") loc.cgiExtension = value;
        else if (token == "cgi_path") loc.cgiPath = value;
        // else loc.directives[token] = value;
        else throw std::runtime_error("Unknown directive in location block: " + token);
    }
}


void ConfigParser::formalizeSpaces(std::string &line)
{
    bool lastWasSpace = true;

    for (std::string::iterator it = line.begin(); it != line.end();)
    {
        if (*it == '\n')
        {
            lastWasSpace = true;
            ++it;
        }
        else if (*it == ' ' || *it == '\t')
        {
            if (lastWasSpace)
            {
                it = line.erase(it);
            }
            else
            {
                lastWasSpace = true;
                ++it;
            }
        }
        else
        {
            lastWasSpace = false;
            ++it;
        }
    }
}

void ConfigParser::findMissingSemicolon(const std::string &text)
{
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it)
    {
        if (*it != '\n')
            continue;

        std::string::const_iterator prev = it;

        if (prev == text.begin())
            continue;

        --prev;

        while (prev != text.begin() && (*prev == ' ' || *prev == '\t'))
            --prev;

        if (*prev == '\n')
            continue;

        if (*prev == ';' || *prev == '{' || *prev == '}')
            continue;

        throw std::runtime_error("Error: missing ';'");
    }
}

void ConfigParser::checkBraces(const std::string &text)
{
    int braceCount = 0;
    int line = 1;

    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it)
    {
        if (*it == '{')
            braceCount++;
        else if (*it == '}')
        {
            braceCount--;
            if (braceCount < 0)
            {
                std::ostringstream oss;
                oss << line;
                throw std::runtime_error(
                    "Error: unexpected '}' at line " + oss.str());
            }
        }

        if (*it == '\n')
            line++;
    }

    if (braceCount != 0)
        throw std::runtime_error("Error: unclosed '{'");
}

void ConfigParser::eraseClosingBraces(std::string &s)
{
    while (!s.empty() &&
          (s[s.size() - 1] == ' ' ||
           s[s.size() - 1] == '\n' ||
           s[s.size() - 1] == '\t'))
    {
        s.erase(s.size() - 1);
    }

    while (!s.empty() && s[s.size() - 1] == '}')
    {
        s.erase(s.size() - 1);

        while (!s.empty() &&
              (s[s.size() - 1] == ' ' ||
               s[s.size() - 1] == '\n' ||
               s[s.size() - 1] == '\t'))
        {
            s.erase(s.size() - 1);
        }
    }
    s += '\n';
    s += '}';
}


void ConfigParser::parseServerBlock(std::istream &input, ServerConfig &server)
{    
    std::string token;
    
    while (input >> token)
    {
        if (token == "}") 
            break;

        if (token == "listen")
        {
            std::string addr;
            std::getline(input, addr, ';');

            size_t start = addr.find_first_not_of(" \t");
            size_t end   = addr.find_last_not_of(" \t");
            if (start != std::string::npos) addr = addr.substr(start, end - start + 1);
            
            size_t colon = addr.find(':');
            if (colon != std::string::npos)
            {
                server.host = addr.substr(0, colon);
                std::string portStr = addr.substr(colon + 1);
                
                if (!isValidIPAddress(server.host))
                    throw std::runtime_error("Error: Invalid IP address '" + server.host + "' in listen directive");
                if (!isValidPortNumber(portStr))
                    throw std::runtime_error("Error: Invalid port number '" + portStr + "' in listen directive");
                
                server.listenPort = ftToInt(portStr);
            }
            else
            {
                if (!isValidPortNumber(addr))
                    throw std::runtime_error("Error: Invalid port number '" + addr + "' in listen directive");
                
                server.host = "0.0.0.0";
                server.listenPort = ftToInt(addr);
            }
        }
        else if (token == "server_name")
        {
            std::string line;
            std::getline(input, line, ';');
            std::stringstream ss(line);
            std::string name;

            while (ss >> name)
            {
                if (!isValidHostname(name))
                    throw std::runtime_error("Error: Invalid server_name '" + name + "'");
                server.serverNames.push_back(name);
            }
        }
        else if (token == "root")
        {
            std::string value;
            std::getline(input, value, ';');
            size_t start = value.find_first_not_of(" \t");
            size_t end   = value.find_last_not_of(" \t");
            if (start != std::string::npos) value = value.substr(start, end - start + 1);
                server.root = value;
        }
        else if (token == "index")
        {
            std::string line;
            std::getline(input, line, ';');
            std::stringstream ss(line);
            std::string file;

            while (ss >> file) 
                server.indexFiles.push_back(file);
        }
        else if (token == "client_max_body_size")
        {
            std::string value;
            std::getline(input, value, ';');
            size_t start = value.find_first_not_of(" \t");
            size_t end   = value.find_last_not_of(" \t");
            if (start != std::string::npos) value = value.substr(start, end - start + 1);

            if (!isValidBodySize(value))
                throw std::runtime_error("Error: Invalid client_max_body_size '" + value + "'");

            char unit = value[value.size() - 1];
            size_t multiplier = 1;
            if (unit == 'K' || unit == 'k') multiplier = 1024, value.resize(value.size() - 1);
            else if (unit == 'M' || unit == 'm') multiplier = 1024*1024, value.resize(value.size() - 1);
            else if (unit == 'G' || unit == 'g') multiplier = 1024*1024*1024, value.resize(value.size() - 1);

            server.clientMaxBodySize = ftToInt(value) * multiplier;
            std::cout << "Set client_max_body_size to : " << server.clientMaxBodySize << " bytes\n";
        }
        else if (token == "error_page")
        {
            std::string line;
            std::getline(input, line, ';');
            std::stringstream ss(line);
            std::string codeStr, path;
            ss >> codeStr >> path;
            int code = ftToInt(codeStr);
            server.errorPages[code] = path;
        }
        else if (token == "location")
        {
            LocationConfig loc;
            input >> loc.path;
            std::string brace;
            input >> brace;
            if (brace != "{") throw std::runtime_error("Expected '{' after location path");
            parseLocationBlock(input, loc);
            server.locations.push_back(loc);
        }
        else throw std::runtime_error("Unknown directive in server block: " + token);
    }
}

void ConfigParser::parseHttpBlock(std::istream &input, HttpConfig &httpConfig)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

        if (token == "server")
        {
            ServerConfig server;
            std::string brace;
            input >> brace;
            if (brace != "{") throw std::runtime_error("Expected '{' after server");
            parseServerBlock(input, server);
            
            // Check for duplicate (host, port) combinations
            for (size_t i = 0; i < httpConfig.servers.size(); ++i) {
                if (httpConfig.servers[i].host == server.host &&
                    httpConfig.servers[i].listenPort == server.listenPort) {
                    std::ostringstream oss;
                    oss << "Error: Duplicate server on " << server.host << ":" << server.listenPort 
                        << " (each server must have unique host:port)";
                    throw std::runtime_error(oss.str());
                }
            }
            
            httpConfig.servers.push_back(server);
        }
        else throw std::runtime_error("Unknown directive inside http block: " + token);
    }
}

HttpConfig ConfigParser::parse()
{
    // Validate syntax before parsing
    std::string tmp = _fileContent;
    formalizeSpaces(tmp);
    eraseClosingBraces(tmp);
    findMissingSemicolon(tmp);
    
    HttpConfig httpConfig;
    std::istringstream config(_fileContent);
    std::string token;

    while (config >> token)
    {
        if (token == "http")
        {
            std::string brace;
            config >> brace;
            if (brace != "{") throw std::runtime_error("Expected '{' after http");
            parseHttpBlock(config, httpConfig);
        }
        else throw std::runtime_error("Unexpected token at root level: " + token);
    }

    // Validate: check for duplicate (host, port) pairs
    std::map<std::pair<std::string, int>, int> endpointCount;
    for (size_t i = 0; i < httpConfig.servers.size(); ++i)
    {
        std::pair<std::string, int> endpoint(httpConfig.servers[i].host, httpConfig.servers[i].listenPort);
        endpointCount[endpoint]++;
        if (endpointCount[endpoint] > 1)
        {
            std::string errMsg = "Error: Duplicate listen address ";
            errMsg += httpConfig.servers[i].host;
            errMsg += ":";
            errMsg += ftToString(httpConfig.servers[i].listenPort);
            errMsg += " found in configuration";
            throw std::runtime_error(errMsg);
        }
    }
    // Validate: check that each server block has a listen directive
    for (size_t i = 0; i < httpConfig.servers.size(); ++i)
    {
        if (httpConfig.servers[i].listenPort == 0)
        {
            throw std::runtime_error("Error: Server block missing required 'listen' directive");
        }
    }

    return httpConfig;
}

// Validation: check if port number is valid
bool ConfigParser::isValidPortNumber(const std::string &portStr)
{
    if (portStr.empty())
        return false;
    
    // Check if string is ALL digits (no letters or special chars)
    for (size_t i = 0; i < portStr.length(); ++i)
    {
        if (!isdigit(portStr[i]))
            return false;
    }
    
    // Convert to int and check range
    try
    {
        int port = ftToInt(portStr);
        if (port < 1 || port > 65535)
            return false;
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}

// Validation: check if hostname/IP is valid format
bool ConfigParser::isValidHostname(const std::string &hostname)
{
    if (hostname.empty())
        return false;
    
    // Basic validation: check for invalid characters
    for (size_t i = 0; i < hostname.length(); ++i)
    {
        char c = hostname[i];
        // Allow alphanumeric, dots, hyphens, underscores
        if (!isalnum(c) && c != '.' && c != '-' && c != '_')
            return false;
    }
    
    return true;
}

// Validation: check if body size value is valid
bool ConfigParser::isValidBodySize(const std::string &sizeStr)
{
    if (sizeStr.empty())
        return false;
    
    // Extract numeric part
    std::string numPart = sizeStr;
    char lastChar = sizeStr[sizeStr.length() - 1];
    
    // If ends with K, M, or G, remove it
    if (lastChar == 'K' || lastChar == 'k' || lastChar == 'M' || lastChar == 'm' 
        || lastChar == 'G' || lastChar == 'g')
    {
        numPart = sizeStr.substr(0, sizeStr.length() - 1);
    }
    
    // Check if numeric part is all digits
    for (size_t i = 0; i < numPart.length(); ++i)
    {
        if (!isdigit(numPart[i]))
            return false;
    }
    
    // Check if numeric part is valid
    if (numPart.empty())
        return false;
    
    try
    {
        int val = ftToInt(numPart);
        if (val <= 0)
            return false;
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}

// Validation: check if IP address is valid IPv4 format
bool ConfigParser::isValidIPAddress(const std::string &ip)
{
    if (ip.empty())
        return false;
    
    // Use inet_pton to validate IPv4 address format
    sockaddr_in sa;
    int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    
    // result > 0 means valid IPv4 address
    return result > 0;
}


