/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:48:25 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:25:41 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "handleCGI.hpp"

#include <sstream>
#include <cstdlib>

HandleCGI::HandleCGI(const HttpRequest& req, const ServerConfig_t& serverConf, const LocationConfig_t& locationConf)
    : _request(req), _serverConf(serverConf), _locationConf(locationConf) {}

HandleCGI::~HandleCGI() {}

void HandleCGI::buildEnv()
{
    _env["REQUEST_METHOD"] = _request.method;
    _env["QUERY_STRING"] = _request.queryString;
    _env["CONTENT_LENGTH"] = ftToString(_request.contentLength);
    _env["CONTENT_TYPE"] = _request.headers.count("Content-Type") ? _request.headers.at("Content-Type") : "";
    _env["SCRIPT_FILENAME"] = _locationConf.root + _request.uri;  // chemin complet vers le script
    _env["SCRIPT_NAME"] = _request.uri;
    _env["SERVER_NAME"] = _serverConf.host;
    _env["SERVER_PORT"] = ftToString(_serverConf.listenPort);
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SERVER_PROTOCOL"] = _request.httpVersion;
    _env["REMOTE_ADDR"] = _request.host;
}

std::vector<char*> HandleCGI::buildEnvArray() const
{
    std::vector<std::string> envStrings;
    for (std::map<std::string, std::string>::const_iterator it = _env.begin();
         it != _env.end(); ++it)
    {
        envStrings.push_back(it->first + "=" + it->second);
    }

    std::vector<char*> envp;
    for (size_t i = 0; i < envStrings.size(); ++i)
        envp.push_back(const_cast<char*>(envStrings[i].c_str()));
    envp.push_back(NULL);

    return envp;
}

std::string HandleCGI::execute()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        std::vector<std::string> envStrings;
        std::vector<char*> envp;
        for (std::map<std::string,std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
        {
            envStrings.push_back(it->first + "=" + it->second);
        }
        for (size_t i = 0; i < envStrings.size(); ++i)
        {
            envp.push_back(const_cast<char*>(envStrings[i].c_str()));
        }
        envp.push_back(NULL);

        char* argv[2];
        argv[0] = const_cast<char*>(_locationConf.cgiPath.c_str());
        argv[1] = NULL;

        execve(argv[0], argv, envp.data());

        perror("execve failed");
        exit(1);
    }
    else
    {
        close(pipefd[1]);

        char buffer[1024];
        std::string output;
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, n);
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        return output;
    }
}
