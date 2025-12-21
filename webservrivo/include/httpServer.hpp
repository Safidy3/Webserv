/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:37:39 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:57:41 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "httpRequest.hpp"
#include "handleErrors.hpp"
#include "utils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <csignal>

struct HttpConfig; 
struct ServerConfig;
struct LocationConfig;
struct MimeTypes;
struct HttpRequest;
struct CGIProcess;
class HttpResponseBuilder;
class HandleErrors;

static const size_t BUFFER_SIZE = 1024;
static const int MAX_PENDING_QUEUE = 10;
static const int MAX_CLIENTS = 100;
static const int CGI_TIMOUT = 10000;
static const int CLIENT_IDLE_TIMEOUT_SEC = 60;
static const int CLIENT_TOTAL_TIMEOUT_SEC = 300;

struct ClientState {
    std::string readBuffer;
    bool headersComplete;
    size_t expectedBody;
    bool chunked;
    time_t lastActivity;
    time_t createdAt;
    size_t receivedBody;
    
    ClientState() : readBuffer(), headersComplete(false), expectedBody(0), chunked(false), lastActivity(0), createdAt(0), receivedBody(0) {}
};

class Server
{
    private:
        const HttpConfig &_config;
        std::vector<struct pollfd> _fds;
        std::map<int, std::vector<const ServerConfig*> > _listenSockets;
        std::vector<int> _clientSockets;
        std::map<int, int> _clientToListenSocket;
        std::map<int, const ServerConfig*> _clientToVirtualServer;
        MimeTypes &_mimeTypes;
        std::map<int, std::string> _sendBuffers;
        std::map<int, bool> _closeAfterSend;
        std::map<int, ClientState> _clients;
        std::map<int, CGIProcess*> _cgiProcesses;
        std::map<pid_t, int> _pidToClientFd;
        

        void setupListeningSockets();
        void handleNewConnection(size_t index);
        void handleClientData(size_t index);
        void saveUploadedFile(const std::string &uploadDir, const std::string &filename, const std::string &fileContent);             
        void handleMultipartUpload(const HttpRequest &req, const std::string &rawRequest, const std::string &uploadDir, int client_fd);
        void queueResponse(int client_fd, const std::string &response);
        void handlePollOut(size_t index);
        void closeClient(size_t index);
        
        bool selectServerForClient(int client_fd, const ClientState &state);
        bool validateAndParseRequest(int client_fd, ClientState &state, HttpRequest &req, const ServerConfig *serverConf);
        bool handleSpecialMethods(int client_fd, const HttpRequest &req, const ServerConfig *serverConf, const LocationConfig *locationConf, const std::string &rawRequest);
        
        void pollCGIProcesses();
        void finalizeCGI(int client_fd, CGIProcess *cgiProc);


    public:
        Server(const HttpConfig &config, MimeTypes &types);
        ~Server();


        void run();
        void cleanup();
};

#endif
