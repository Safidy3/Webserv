#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"

class CGIHandler
{
public:
    CGIHandler();
    ~CGIHandler();

    std::string execute(const HTTPRequest&, const ServerConfig&); // (fork(), execve(), pipes).
    void        setEnv(const HTTPRequest&, const ServerConfig&);
};

#endif
