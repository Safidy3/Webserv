/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleErrors.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 19:25:19 by rhanitra          #+#    #+#             */
/*   Updated: 2025/11/26 15:37:50 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLE_ERROR_HPP
#define HANDLE_ERROR_HPP

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include "httpConfig.hpp"

struct ServerConfig;
struct LocationConfig;

class HandleErrors
{
    private:
        static std::map<int, std::string> initReasonMap();
        static std::string getDefaultReason(int code);
        static std::string getErrorBodyFromFile(const std::string &filePath, int code, const std::string &reason);
        
    public:
        static std::string generateErrorResponse(int code, const ServerConfig &serverConf, 
            const LocationConfig *locationConf = NULL, const std::string &extraHeaders = "");
};

#endif

