/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:47:11 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 13:22:50 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLE_CGI_HPP
#define HANDLE_CGI_HPP

#include "httpRequest.hpp"
#include "httpConfig.hpp"
#include "CgiTimeout.hpp"
#include "utils.hpp"
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

struct CGIProcess {
    pid_t pid;
    int pipe_out;
    int pipe_err;
    std::string output;
    std::string error;
    bool out_eof;
    bool err_eof;
    time_t startTime;
    int timeoutMs;
    bool timedOut;
    
    CGIProcess() : pid(-1), pipe_out(-1), pipe_err(-1), output(), error(), 
                   out_eof(false), err_eof(false), startTime(0), timeoutMs(CGI_TIMOUT), timedOut(false) {}
};



class HandleCGI
{
    private:
        const HttpRequest& _request;
        const ServerConfig& _serverConf;
        const LocationConfig& _locationConf;

        std::map<std::string, std::string> _env;

    public:
        HandleCGI(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &_locationConf);
        ~HandleCGI();

        void printEnv() const;
        void buildEnv();
        std::vector<std::string> buildEnvStrings() const;
        
        CGIProcess* execute();
        
        static std::string readCGIOutput(CGIProcess *cgiProc);
};

#endif
