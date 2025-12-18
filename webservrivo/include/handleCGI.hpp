/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:47:11 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/18 09:33:01 by rivoinfo         ###   ########.fr       */
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

// Structure pour tracker un CGI en cours
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
                   out_eof(false), err_eof(false), startTime(0), timeoutMs(10000), timedOut(false) {}
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
        std::string execute();
        
        // Asynchrone version - starts CGI without waiting
        CGIProcess* executeAsync();
        
        // Helper to read from CGI pipes
        static std::string readCGIOutput(CGIProcess *cgiProc);
};

#endif
