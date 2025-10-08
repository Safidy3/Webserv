#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "../webserv.hpp"

class ErrorHandler
{
    private:
        static std::map<int, std::string>	initReasonMap();
        static std::string					getDefaultReason(int code);
        static std::string					getErrorBodyFromFile(const std::string &filePath, int code, const std::string &reason);

    public:
        static std::string	generateErrorResponse(int code, const ServerConfig_t &serverConf, 
			const LocationConfig_t *locationConf = NULL, const std::string &extraHeaders = "");

};

#endif
