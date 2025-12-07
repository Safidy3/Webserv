/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:48:25 by rhanitra          #+#    #+#             */
/*   Updated: 2025/11/27 16:18:38 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/handleCGI.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>

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

std::vector<std::string> HandleCGI::buildEnvStrings() const
{
    std::vector<std::string> envStrings;
    for (std::map<std::string, std::string>::const_iterator it = _env.begin();
         it != _env.end(); ++it)
    {
        envStrings.push_back(it->first + "=" + it->second);
    }
    return envStrings;
}

std::string HandleCGI::execute()
{
    int pipe_out[2]; // pour stdout du CGI -> parent lit pipe_out[0]
    int pipe_err[2]; // pour stderr du CGI -> parent lit pipe_err[0]
    int pipe_in[2];  // pour stdin du CGI <- parent écrit pipe_in[1]
    if (pipe(pipe_out) == -1 || pipe(pipe_err) == -1 || pipe(pipe_in) == -1)
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

    // redirect stdout -> pipe_out[1], stderr -> pipe_err[1]
    if (dup2(pipe_out[1], STDOUT_FILENO) == -1) perror("dup2 stdout");
    if (dup2(pipe_err[1], STDERR_FILENO) == -1) perror("dup2 stderr");

        // redirect stdin <- pipe_in[0]
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) perror("dup2 stdin");

    // close now duplicated fds
    close(pipe_out[1]);
    close(pipe_err[1]);
    close(pipe_in[0]);

        // --- Construire envp ---
        // Build std::string vector first so the data remains valid until execve()
        std::vector<std::string> envStrings = buildEnvStrings();

        // s'assurer de REDIRECT_STATUS=200
        bool hasRedirect = false;
        for (size_t i = 0; i < envStrings.size(); ++i) {
            if (envStrings[i].rfind("REDIRECT_STATUS=", 0) == 0) { hasRedirect = true; break; }
        }
        if (!hasRedirect) envStrings.push_back("REDIRECT_STATUS=200");

        std::vector<char*> envp;
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
        // parent n'a pas besoin des extrémités d'enfant (write ends de out/err)
        close(pipe_out[1]);
        close(pipe_err[1]);
        close(pipe_in[0]);

        // Si la requête a un body (POST), on l'envoie au stdin du child
        if (!_request.body.empty()) {
            ssize_t toWrite = _request.body.size();
            const char* data = _request.body.data();
            while (toWrite > 0) {
                ssize_t w = write(pipe_in[1], data, toWrite);
                if (w < 0) {
                    if (errno == EINTR) continue;
                    break;
                }
                toWrite -= w;
                data += w;
            }
        }
        // fermer la pipe stdin pour indiquer EOF au child
        close(pipe_in[1]);

        // Lire la sortie stdout et stderr du child simultanément
        std::string out;
        std::string err;
        const int fd_out = pipe_out[0];
        const int fd_err = pipe_err[0];
        struct pollfd fds[2];
        fds[0].fd = fd_out; fds[0].events = POLLIN;
        fds[1].fd = fd_err; fds[1].events = POLLIN;

        bool out_eof = false, err_eof = false;
        char buffer[4096];

        // Timeout for CGI execution in milliseconds
        // Use per-location setting if provided, otherwise fallback to 10s
        int timeoutMs = 10000;
        if (_locationConf.cgiTimeoutSeconds > 0) {
            // beware of overflow: multiply as int
            timeoutMs = _locationConf.cgiTimeoutSeconds * 1000;
        }
        struct timeval tv_start, tv_now;
        gettimeofday(&tv_start, NULL);

        while (!(out_eof && err_eof)) {
            // compute remaining time
            gettimeofday(&tv_now, NULL);
            long elapsed = (tv_now.tv_sec - tv_start.tv_sec) * 1000L + (tv_now.tv_usec - tv_start.tv_usec) / 1000L;
            int remaining = timeoutMs - static_cast<int>(elapsed);
            if (remaining <= 0) {
                // timeout: kill child and cleanup
                std::cerr << "CGI timeout: killing child pid " << pid << std::endl;
                kill(pid, SIGKILL);
                // close fds to break reads
                close(fd_out);
                close(fd_err);
                // reap child
                int status = 0;
                waitpid(pid, &status, 0);
                throw std::runtime_error("handleCGI: child timed out");
            }

            int ret = poll(fds, 2, remaining);
            if (ret < 0) {
                if (errno == EINTR) continue;
                break;
            }
            if (ret == 0) {
                // poll timeout loop will check elapsed and kill if needed
                continue;
            }

            // stdout
            if (!out_eof && (fds[0].revents & POLLIN)) {
                ssize_t n = read(fd_out, buffer, sizeof(buffer));
                if (n > 0) out.append(buffer, n);
                else if (n == 0) out_eof = true;
                else if (errno == EINTR) continue;
                else out_eof = true;
            }
            if (!out_eof && (fds[0].revents & (POLLHUP | POLLERR))) out_eof = true;

            // stderr
            if (!err_eof && (fds[1].revents & POLLIN)) {
                ssize_t n = read(fd_err, buffer, sizeof(buffer));
                if (n > 0) err.append(buffer, n);
                else if (n == 0) err_eof = true;
                else if (errno == EINTR) continue;
                else err_eof = true;
            }
            if (!err_eof && (fds[1].revents & (POLLHUP | POLLERR))) err_eof = true;
        }

        close(fd_out);
        close(fd_err);

        // Reaper le child
        int status = 0;
        waitpid(pid, &status, 0);

        // If there is any stderr output or non-zero exit, log and throw
        if (!err.empty()) {
            std::cerr << "CGI stderr: " << err << std::endl;
            throw std::runtime_error("handleCGI: child produced stderr");
        }
        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                std::ostringstream oss;
                oss << "handleCGI: child exited with code " << exitCode;
                throw std::runtime_error(oss.str());
            }
        } else if (WIFSIGNALED(status)) {
            std::ostringstream oss;
            oss << "handleCGI: child terminated by signal " << WTERMSIG(status);
            throw std::runtime_error(oss.str());
        }

        return out;
    }
}
