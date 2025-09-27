/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 13:30:58 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/16 16:19:42 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP

#include "utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

struct MimeTypes
{
    std::map<std::string, std::string> types;
};

struct LocationConfig
{
    std::string path;
    std::string root;
    std::vector<std::string> indexFiles;
    std::vector<std::string> methods;
    bool autoindex;
    int returnCode;
    std::string returnPath;
    std::string cgiExtension;
    std::string cgiPath;
    std::string uploadDir;
    std::string defaultFile;
    std::map<std::string,std::string> directives;

    LocationConfig() : autoindex(false), returnCode(0) {}
};

struct ServerConfig
{
    std::string host;
    int listenPort;
    std::string root;
    std::vector<std::string> indexFiles;
    size_t clientMaxBodySize;
    std::map<int,std::string> errorPages;
    std::vector<LocationConfig> locations;

    ServerConfig() : listenPort(0), clientMaxBodySize(0) {}
};

struct HttpConfig
{
    std::vector<ServerConfig> servers;
};

class ConfigParser
{
    private:
        std::string _configFilePath;
        std::string _mimeTypesPath;
        std::string _fileContent;
        // MimeTypes &_mineTypes;
        
        
        void expectToken(std::istream &input, const std::string &expected);
        void parseHttpBlock(std::istream &input, HttpConfig &httpConfig);
        void parseServerBlock(std::istream &input, ServerConfig &config);
        void parseLocationBlock(std::istream &input, LocationConfig &loc);

    public:
        ConfigParser(const std::string &_configFilePath, const std::string &mimeTypesPath);
        ConfigParser(const ConfigParser& other);
        ConfigParser& operator=(const ConfigParser& other);
        ~ConfigParser();

        void loadMimeTypes(MimeTypes &mimeTypes);
        HttpConfig parse();

};

#endif
