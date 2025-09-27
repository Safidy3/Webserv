/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:48:25 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/22 19:28:23 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/handleCGI.hpp"

HandleCGI::HandleCGI(const HttpRequest& req, const ServerConfig& serverConf, const LocationConfig& locationConf)
    : _request(req), _serverConf(serverConf), _locationConf(locationConf) {}

HandleCGI::~HandleCGI() {}

void HandleCGI::printEnv() const {
    for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it) {
        std::cout << it->first << " = " << it->second << std::endl;
    }
}


void HandleCGI::buildEnv()
{
    _env["REQUEST_METHOD"] = _request.method;
    _env["QUERY_STRING"] = _request.queryString;
    _env["CONTENT_LENGTH"] = ftToString(_request.contentLength);
    _env["CONTENT_TYPE"] = _request.headers.count("Content-Type") ? _request.headers.at("Content-Type") : "";
    _env["SCRIPT_FILENAME"] = _locationConf.root + _request.uri.substr(_locationConf.path.size());
    _env["SCRIPT_NAME"] = _request.uri;
    _env["SERVER_NAME"] = _serverConf.host;
    _env["SERVER_PORT"] = ftToString(_serverConf.listenPort);
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SERVER_PROTOCOL"] = _request.httpVersion;
    _env["REMOTE_ADDR"] = _request.host;
    _env["REDIRECT_STATUS"] = "200";

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
    int pipe_out[2]; // pour stdout/stderr du CGI -> parent lit pipe_out[0]
    int pipe_in[2];  // pour stdin du CGI <- parent écrit pipe_in[1]
    if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0) // child
    {
        // --- Child: préparer les redirections ---
        // ferme les extrémités non utilisées
        close(pipe_out[0]); // child n'a pas besoin de read end for stdout
        close(pipe_in[1]);  // child n'a pas besoin de write end for stdin

        // redirect stdout & stderr -> pipe_out[1]
        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) perror("dup2 stdout");
        if (dup2(pipe_out[1], STDERR_FILENO) == -1) perror("dup2 stderr");

        // redirect stdin <- pipe_in[0]
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) perror("dup2 stdin");

        // close now duplicated fds
        close(pipe_out[1]);
        close(pipe_in[0]);

        // --- Construire envp ---
        std::vector<std::string> envStrings;
        std::vector<char*> envp;
        for (std::map<std::string,std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
            envStrings.push_back(it->first + "=" + it->second);

        // s'assurer de REDIRECT_STATUS=200
        bool hasRedirect = false;
        for (size_t i = 0; i < envStrings.size(); ++i) {
            if (envStrings[i].rfind("REDIRECT_STATUS=", 0) == 0) { hasRedirect = true; break; }
        }
        if (!hasRedirect) envStrings.push_back("REDIRECT_STATUS=200");

        envp.reserve(envStrings.size() + 1);
        for (size_t i = 0; i < envStrings.size(); ++i)
            envp.push_back(const_cast<char*>(envStrings[i].c_str()));
        envp.push_back(NULL);

        // --- Construire argv proprement (EVITER pointer sur std::string temporaire) ---
        std::string scriptPath = _locationConf.root + _request.uri.substr(_locationConf.path.size());
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(_locationConf.cgiPath.c_str())); // program path
        argv.push_back(const_cast<char*>(scriptPath.c_str()));           // script path
        argv.push_back(NULL);

        // --- Execve ---
        execve(argv[0], &argv[0], &envp[0]);

        // Si execve échoue
        perror("execve failed");
        _exit(127); // use _exit to avoid flushing parent's buffers twice
    }
    else // parent
    {
        // parent n'a pas besoin des extrémités d'enfant
        close(pipe_out[1]);
        close(pipe_in[0]);

        // Si la requête a un body (POST), on l'envoie au stdin du child
        if (!_request.body.empty()) {
            ssize_t toWrite = _request.body.size();
            const char* data = _request.body.data();
            while (toWrite > 0) {
                ssize_t w = write(pipe_in[1], data, toWrite);
                if (w <= 0) {
                    if (errno == EINTR) continue;
                    break;
                }
                toWrite -= w;
                data += w;
            }
        }
        // fermer la pipe stdin pour indiquer EOF au child
        close(pipe_in[1]);

        // Lire la sortie du child
        std::string output;
        char buffer[4096];
        ssize_t n;
        while ((n = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, n);
            // Optionnel : envoyer directement au client ici plutôt que stocker
        }
        close(pipe_out[0]);

        // Reaper le child
        int status = 0;
        waitpid(pid, &status, 0);

        return output;
    }
}


/*std::string HandleCGI::execute()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0) // Processus fils
    {
        // fermer l'extrémité lecture du pipe
        close(pipefd[0]);

        // rediriger stdout et stderr vers le pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Construire envp
        std::vector<std::string> envStrings;
        std::vector<char*> envp;
        for (std::map<std::string,std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
        {
            envStrings.push_back(it->first + "=" + it->second);
        }
        for (size_t i = 0; i < envStrings.size(); ++i)
            envp.push_back(const_cast<char*>(envStrings[i].c_str()));
        envp.push_back(NULL);

        // Construire argv
        char* argv[3];
        argv[0] = const_cast<char*>(_locationConf.cgiPath.c_str());              // ex: /usr/bin/php-cgi
        argv[1] = const_cast<char*>((_locationConf.root + _request.uri.substr(_locationConf.path.size())).c_str());
        argv[2] = NULL;

        // Exécution
        execve(argv[0], argv, envp.data());

        // Si execve échoue
        perror("execve failed");
        exit(1);
    }
    else // Processus parent
    {
        close(pipefd[1]);

        char buffer[1024];
        std::string output;
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0)
            output.append(buffer, n);
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        return output;
    }
}*/
