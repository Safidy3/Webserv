/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:37:39 by rhanitra          #+#    #+#             */
/*   Updated: 2025/11/26 13:53:35 by rivoinfo         ###   ########.fr       */
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
class HttpResponseBuilder;
class HandleErrors;

static const size_t BUFFER_SIZE = 1024;
static const int MAX_PENDING_QUEUE = 10;
static const int MAX_CLIENTS = 100;


class Server
{
    private:
        const HttpConfig &_config;
        std::vector<struct pollfd> _fds;
        std::vector<int> _clientSockets;
        std::map<int, std::vector<const ServerConfig*> > _listenSockets;
        std::map<int, const ServerConfig*> _clientToServer;
        std::map<int, int> _clientToListenSocket;  // map client_fd -> listening_socket_fd
        MimeTypes &_mimeTypes;
        
        // Outgoing send buffers for non-blocking writes
        std::map<int, std::string> _sendBuffers;
        std::map<int, bool> _closeAfterSend;
        
        // Per-client read buffers and parsing state
        struct ClientState {
            std::string readBuffer;
            bool headersComplete;
            size_t expectedBody;
            bool chunked;
            time_t lastActivity;
            time_t createdAt;
            ClientState(): readBuffer(), headersComplete(false), expectedBody(0), chunked(false), lastActivity(0), createdAt(0) {}
        };
        std::map<int, ClientState> _clients;
        static const int CLIENT_IDLE_TIMEOUT_SEC = 60; // close after 60s idle
        static const int CLIENT_TOTAL_TIMEOUT_SEC = 300; // max 5 minutes per connection

        void setupListeningSockets();
        void handleNewConnection(size_t index);
        void handleClientData(size_t index);
        void saveUploadedFile(const std::string &uploadDir, const std::string &filename, const std::string &fileContent);             
        void handleMultipartUpload(const HttpRequest &req, const std::string &rawRequest, const std::string &uploadDir, int client_fd);
        void queueResponse(int client_fd, const std::string &response);
        void handlePollOut(size_t index);
        void closeClient(size_t index);


    public:
        Server(const HttpConfig &config, MimeTypes &types);
        ~Server();


        void run();
        void cleanup();
};

#endif
