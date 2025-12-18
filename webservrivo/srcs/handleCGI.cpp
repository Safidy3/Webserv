/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:48:25 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/18 09:01:06 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/handleCGI.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>
#include <cstring>

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
    int pipe_out[2]; // For CGI stdout -> parent reads pipe_out[0]
    int pipe_err[2]; // For CGI stderr -> parent reads pipe_err[0]
    int pipe_in[2];  // For CGI stdin <- parent writes pipe_in[1]
    if (pipe(pipe_out) == -1 || pipe(pipe_err) == -1 || pipe(pipe_in) == -1)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0) // child
    {
        // --- Child: Setup redirections ---
        // Close unused ends
        close(pipe_out[0]); // child doesn't need read end for stdout
        close(pipe_in[1]);  // child doesn't need write end for stdin

        // Redirect stdout -> pipe_out[1], stderr -> pipe_err[1]
        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) perror("dup2 stdout");
        if (dup2(pipe_err[1], STDERR_FILENO) == -1) perror("dup2 stderr");

        // Redirect stdin <- pipe_in[0]
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) perror("dup2 stdin");

        // Close now duplicated file descriptors
        close(pipe_out[1]);
        close(pipe_err[1]);
        close(pipe_in[0]);

        // --- Build environment variables ---
        // Build std::string vector first so the data remains valid until execve()
        std::vector<std::string> envStrings = buildEnvStrings();

        // Ensure REDIRECT_STATUS=200
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

        // --- Build argv properly ---
        std::string scriptPath = _locationConf.root + _request.uri.substr(_locationConf.path.size());
        std::string cgiPath = _locationConf.cgiPath;
        
        // Create argv arrays
        const char* args[3];
        args[0] = cgiPath.c_str();
        args[1] = scriptPath.c_str();
        args[2] = NULL;

        // --- Execute CGI ---
        execve(args[0], (char* const*)args, &envp[0]);

        // If execve fails
        perror("execve failed");
        _exit(127); // use _exit to avoid flushing parent's buffers twice
    }
    else // parent
    {
        // Parent doesn't need child's write ends (of stdout/stderr)
        close(pipe_out[1]);
        close(pipe_err[1]);
        close(pipe_in[0]);

        // If the request has a body (POST), send it to the child's stdin
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
        // Close stdin pipe to signal EOF to the child
        close(pipe_in[1]);

        // Read stdout and stderr from child simultaneously
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
            // Beware of overflow: multiply as int
            timeoutMs = _locationConf.cgiTimeoutSeconds * 1000;
        }
        struct timeval tv_start, tv_now;
        gettimeofday(&tv_start, NULL);

        while (!(out_eof && err_eof)) {
            // Compute remaining time
            gettimeofday(&tv_now, NULL);
            long elapsed = (tv_now.tv_sec - tv_start.tv_sec) * 1000L + (tv_now.tv_usec - tv_start.tv_usec) / 1000L;
            int remaining = timeoutMs - static_cast<int>(elapsed);
            if (remaining <= 0) {
                std::cerr << "CGI timeout: killing child pid " << pid << std::endl;
                kill(pid, SIGKILL);
                close(fd_out);
                close(fd_err);
                waitpid(pid, NULL, 0);
                throw CgiTimeout();
            }
            int ret = poll(fds, 2, remaining);
            if (ret < 0) {
                if (errno == EINTR) continue;
                break;
            }
            if (ret == 0) {
                // Poll timeout; loop will check elapsed and kill if needed
                continue;
            }

            // Read stdout
            if (!out_eof && (fds[0].revents & POLLIN)) {
                ssize_t n = read(fd_out, buffer, sizeof(buffer));
                if (n > 0) out.append(buffer, n);
                else if (n == 0) out_eof = true;
                else if (errno == EINTR) continue;
                else out_eof = true;
            }
            if (!out_eof && (fds[0].revents & (POLLHUP | POLLERR))) out_eof = true;

            // Read stderr
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

        // Reap the child
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

// Asynchronous CGI execution - fork without waiting
CGIProcess* HandleCGI::executeAsync()
{
    int pipe_out[2];
    int pipe_err[2];
    int pipe_in[2];
    
    if (pipe(pipe_out) == -1 || pipe(pipe_err) == -1 || pipe(pipe_in) == -1) {
        std::cerr << "executeAsync: pipe failed\n";
        return NULL;
    }

    // Extract string data into C buffers BEFORE fork to avoid heap issues
    // Note: We access _locationConf and _request BEFORE fork - they're invalid after fork in child
    char cgiPath_buf[512];
    char scriptPath_buf[512];
    char body_buf[4096];
    size_t body_size = 0;
    int timeout_val = 10000;
    
    {
        // Copy CGI path
        const char *path = _locationConf.cgiPath.c_str();
        size_t len = _locationConf.cgiPath.length();
        if (len >= sizeof(cgiPath_buf)) len = sizeof(cgiPath_buf) - 1;
        memcpy(cgiPath_buf, path, len);
        cgiPath_buf[len] = '\0';
        
        // Copy script path (reconstructed)
        std::string fullScriptPath = _locationConf.root + _request.uri.substr(_locationConf.path.size());
        const char *spath = fullScriptPath.c_str();
        size_t slen = fullScriptPath.length();
        if (slen >= sizeof(scriptPath_buf)) slen = sizeof(scriptPath_buf) - 1;
        memcpy(scriptPath_buf, spath, slen);
        scriptPath_buf[slen] = '\0';
        
        // Copy body
        if (!_request.body.empty()) {
            const char *bdata = _request.body.data();
            size_t blen = _request.body.length();
            if (blen >= sizeof(body_buf)) blen = sizeof(body_buf) - 1;
            memcpy(body_buf, bdata, blen);
            body_buf[blen] = '\0';
            body_size = blen;
        }
        
        // Copy timeout
        timeout_val = _locationConf.cgiTimeoutSeconds > 0 ? (_locationConf.cgiTimeoutSeconds * 1000) : 10000;
    }
    
    // Build environment BEFORE fork
    buildEnv();
    std::vector<std::string> envStrings = buildEnvStrings();
    bool hasRedirect = false;
    for (size_t i = 0; i < envStrings.size(); ++i) {
        if (envStrings[i].rfind("REDIRECT_STATUS=", 0) == 0) { hasRedirect = true; break; }
    }
    if (!hasRedirect) envStrings.push_back("REDIRECT_STATUS=200");

    // Create C-style arrays BEFORE fork
    std::vector<char*> envp_data;
    envp_data.reserve(envStrings.size() + 1);
    for (size_t i = 0; i < envStrings.size(); ++i)
        envp_data.push_back(const_cast<char*>(envStrings[i].c_str()));
    envp_data.push_back(NULL);

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "executeAsync: fork failed\n";
        close(pipe_out[0]); close(pipe_out[1]);
        close(pipe_err[0]); close(pipe_err[1]);
        close(pipe_in[0]);  close(pipe_in[1]);
        return NULL;
    }

    if (pid == 0) // child - DO NOT USE any parent member variables here!
    {
        close(pipe_out[0]);
        close(pipe_in[1]);

        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) perror("dup2 stdout");
        if (dup2(pipe_err[1], STDERR_FILENO) == -1) perror("dup2 stderr");
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) perror("dup2 stdin");

        close(pipe_out[1]);
        close(pipe_err[1]);
        close(pipe_in[0]);

        // Use ONLY the C buffers which are on stack and guaranteed to be valid
        char *argv[3];
        argv[0] = cgiPath_buf;
        argv[1] = scriptPath_buf;
        argv[2] = NULL;

        // Call execve with environment
        execve(argv[0], argv, &envp_data[0]);
        
        // If execve fails
        perror("execve failed");
        _exit(127);
    }
    else // parent
    {
        close(pipe_out[1]);
        close(pipe_err[1]);
        close(pipe_in[0]);

        // Write request body if any
        if (body_size > 0) {
            ssize_t toWrite = body_size;
            const char* data = body_buf;
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
        close(pipe_in[1]);

        CGIProcess *cgiProc = new CGIProcess();
        cgiProc->pid = pid;
        cgiProc->pipe_out = pipe_out[0];
        cgiProc->pipe_err = pipe_err[0];
        cgiProc->out_eof = false;
        cgiProc->err_eof = false;
        cgiProc->startTime = time(NULL);
        cgiProc->timeoutMs = timeout_val;

        int flags = fcntl(pipe_out[0], F_GETFL, 0);
        fcntl(pipe_out[0], F_SETFL, flags | O_NONBLOCK);
        flags = fcntl(pipe_err[0], F_GETFL, 0);
        fcntl(pipe_err[0], F_SETFL, flags | O_NONBLOCK);

        return cgiProc;
    }
}

// Read available data from CGI process (non-blocking)
std::string HandleCGI::readCGIOutput(CGIProcess *cgiProc)
{
    if (!cgiProc) return std::string();
    
    char buffer[4096];
    
    // Try to read from stdout
    if (!cgiProc->out_eof && cgiProc->pipe_out >= 0) {
        ssize_t n = read(cgiProc->pipe_out, buffer, sizeof(buffer));
        if (n > 0) {
            cgiProc->output.append(buffer, n);
        } else if (n == 0) {
            cgiProc->out_eof = true;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now
        } else {
            cgiProc->out_eof = true;
        }
    }
    
    // Try to read from stderr
    if (!cgiProc->err_eof && cgiProc->pipe_err >= 0) {
        ssize_t n = read(cgiProc->pipe_err, buffer, sizeof(buffer));
        if (n > 0) {
            cgiProc->error.append(buffer, n);
        } else if (n == 0) {
            cgiProc->err_eof = true;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now
        } else {
            cgiProc->err_eof = true;
        }
    }
    
    return cgiProc->output;
}
