/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:10:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/16 17:17:09 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpConfig.hpp"

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
            loc.returnCode = 301;
            loc.returnPath = value;
        }
        else if (token == "default_file") loc.defaultFile = value;
        else if (token == "cgi_extension") loc.cgiExtension = value;
        else if (token == "cgi_path") loc.cgiPath = value;
        else loc.directives[token] = value;
    }
}

void ConfigParser::parseServerBlock(std::istream &input, ServerConfig &server)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

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
                server.listenPort = ftToInt(addr.substr(colon + 1));
            }
            else
            {
                server.host = "0.0.0.0";
                server.listenPort = ftToInt(addr);
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
            while (ss >> file) server.indexFiles.push_back(file);
        }
        else if (token == "client_max_body_size")
        {
            std::string value;
            std::getline(input, value, ';');
            size_t start = value.find_first_not_of(" \t");
            size_t end   = value.find_last_not_of(" \t");
            if (start != std::string::npos) value = value.substr(start, end - start + 1);

            char unit = value[value.size() - 1];
            size_t multiplier = 1;
            if (unit == 'K' || unit == 'k') multiplier = 1024, value.resize(value.size() - 1);
            else if (unit == 'M' || unit == 'm') multiplier = 1024*1024, value.resize(value.size() - 1);
            else if (unit == 'G' || unit == 'g') multiplier = 1024*1024*1024, value.resize(value.size() - 1);

            server.clientMaxBodySize = ftToInt(value) * multiplier;
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
            httpConfig.servers.push_back(server);
        }
        else throw std::runtime_error("Unknown directive inside http block: " + token);
    }
}

HttpConfig ConfigParser::parse()
{
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
    return httpConfig;
}

