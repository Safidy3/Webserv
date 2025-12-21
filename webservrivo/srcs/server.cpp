/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:58:26 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpServer.hpp"
#include "../include/httpResponse.hpp"
#include "../include/httpConfig.hpp"
#include "../include/handleCGI.hpp"
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include "../include/utils.hpp"
#include <fcntl.h>
#include <arpa/inet.h>

Server::Server(const HttpConfig &config, MimeTypes &types)
    : _config(config), _mimeTypes(types)
{
    setupListeningSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < _clientSockets.size(); ++i)
        close(_clientSockets[i]);
    for (size_t i = 0; i < _fds.size(); ++i)
        close(_fds[i].fd);
}

void Server::setupListeningSockets()
{
    // Group servers by (host, port) to create ONE socket per endpoint
    std::map<std::pair<std::string, int>, std::vector<size_t> > serversByEndpoint;
    for (size_t i = 0; i < _config.servers.size(); ++i)
    {
        std::pair<std::string, int> endpoint(_config.servers[i].host, _config.servers[i].listenPort);
        serversByEndpoint[endpoint].push_back(i);
    }

    // For each unique endpoint, create a socket and add all servers to it
    for (std::map<std::pair<std::string, int>, std::vector<size_t> >::iterator it = serversByEndpoint.begin();
         it != serversByEndpoint.end(); ++it) {
        const std::pair<std::string, int> &endpoint = it->first;
        const std::vector<size_t> &serverIndices = it->second;
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            std::cerr << "Error: socket init failed\n";
            continue;
        }

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(endpoint.second);
        
        // Validate and convert IP address
        int inet_result = inet_pton(AF_INET, endpoint.first.c_str(), &addr.sin_addr);
        if (inet_result <= 0) {
            std::cerr << "Error: Invalid IP address '" << endpoint.first << "' (errno=" << errno << ")\n";
            close(sock);
            continue;
        }

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            std::cerr << "Error: bind failed on " << endpoint.first << ":" << endpoint.second
                      << " (errno=" << errno << ") " << strerror(errno) << "\n";
            close(sock);
            continue;
        }

        if (listen(sock, MAX_PENDING_QUEUE) == -1) {
            std::cerr << "Error: listen failed on port " << endpoint.second
                      << " (errno=" << errno << ") " << strerror(errno) << "\n";
            close(sock);
            continue;
        }

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _fds.push_back(pfd);

        // Add ALL servers of this endpoint
        for (size_t j = 0; j < serverIndices.size(); ++j) {
            _listenSockets[sock].push_back(&_config.servers[serverIndices[j]]);
        }
        
        std::cout << "Listening on " << endpoint.first << ":" << endpoint.second << std::endl;
    }
}

void Server::handleNewConnection(size_t index)
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_sock = accept(_fds[index].fd, (sockaddr*)&client_addr, &addrlen);
    if (client_sock == -1) {
        std::cerr << "Error: accept failed (errno=" << errno << ")\n";
        return;
    }
    // Set client socket to non-blocking
    int flags = fcntl(client_sock, F_GETFL, 0);
    fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
    struct pollfd client_pfd;
    client_pfd.fd = client_sock;
    client_pfd.events = POLLIN;
    client_pfd.revents = 0;
    _fds.push_back(client_pfd);
        _clientSockets.push_back(client_sock);
    // Initialize client parsing state
        _clients[client_sock] = ClientState();
        time_t now = time(NULL);
        _clients[client_sock].lastActivity = now;
        _clients[client_sock].createdAt = now;

        // Store the listening socket to select the server later
        // We will store the listening fd index in a temporary map
        // to retrieve the list of possible servers
        if (_listenSockets.count(_fds[index].fd)) {
            // We don't select the server now
            // handleClientData() will do it based on Host: header
            _clientToVirtualServer[client_sock] = NULL;  // To be determined later
            _clientToListenSocket[client_sock] = _fds[index].fd;  // Store listen fd
            std::cout << "New client " << client_sock 
                      << " connected (server selection pending Host: header)\n";
        } else {
            std::cerr << "Error: listening socket " << _fds[index].fd 
                      << " not found in _listenSockets!" << std::endl;
        }
}





void Server::handleMultipartUpload(const HttpRequest &req, const std::string &rawRequest, const std::string &uploadDir, int client_fd)

{
    std::string contentType;
    if (req.headers.find("Content-Type") != req.headers.end())
        contentType = req.headers.find("Content-Type")->second;

    if (contentType.find("multipart/form-data") == std::string::npos)
        return;

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return;

    std::string boundary = contentType.substr(pos + 9);
    size_t bodyStart = rawRequest.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
        return;
    std::string body = rawRequest.substr(bodyStart + 4);
    std::string delimiter = "--" + boundary;

    std::vector<std::string> parts = ftSplit(body, delimiter);

    std::map<std::string, std::string> fields;
    std::string uploadedFilename;

    for (size_t i = 0; i < parts.size(); ++i)
    {
        std::string name = extractFieldName(parts[i]);
        std::string filename = extractFilename(parts[i]);
        std::string data = extractFileContent(parts[i]);

        if (!filename.empty()) {
            saveUploadedFile(uploadDir, filename, data);
            uploadedFilename = filename;
        } else if (!name.empty()) {
            fields[name] = data;
        }
    }

    // Ajout du fichier CSV
    appendToCSV(fields, uploadDir + "/contacts.csv", uploadedFilename);

    std::string response =
        "HTTP/1.0 201 Created\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

    queueResponse(client_fd, response);
}


void Server::saveUploadedFile(const std::string &uploadDir,
                      const std::string &filename,
                      const std::string &fileContent)
{
    std::string fullPath = uploadDir + "/" + filename;

    size_t slash = fullPath.find_last_of('/');
    if (slash != std::string::npos) {
        std::string dir = fullPath.substr(0, slash);
        struct stat st;
        if (stat(dir.c_str(), &st) == -1) {
            std::string accum;
            size_t pos = 0;
            if (dir.size() > 0 && dir[0] == '/') { accum = "/"; pos = 1; }
            while (pos <= dir.size()) {
                size_t next = dir.find('/', pos);
                if (next == std::string::npos) next = dir.size();
                std::string part = dir.substr(pos, next - pos);
                if (!part.empty()) {
                    if (accum.size() > 1 && accum[accum.size()-1] != '/') accum += "/";
                    accum += part;
                    if (stat(accum.c_str(), &st) == -1) {
                        if (mkdir(accum.c_str(), 0755) == -1 && errno != EEXIST) {
                            std::cerr << "saveUploadedFile: failed to create dir " << accum << "\n";
                            return;
                        }
                    }
                }
                pos = next + 1;
            }
        }
    }
    std::ofstream out(fullPath.c_str(), std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "saveUploadedFile: cannot open " << fullPath << " for writing\n";
        return;
    }
    out.write(fileContent.c_str(), fileContent.size());
    out.close();
}

// Select the appropriate server for a client based on Host header
bool Server::selectServerForClient(int client_fd, const ClientState &state)
{
    if (_clientToListenSocket.count(client_fd) == 0)
        return false;

    int listen_fd = _clientToListenSocket[client_fd];
    const std::vector<const ServerConfig*> &candidates = _listenSockets[listen_fd];

    std::string requestHost;
    size_t hostPos = state.readBuffer.find("Host:");
    if (hostPos != std::string::npos) {
        size_t hostStart = hostPos + 5;
        while (hostStart < state.readBuffer.size() && (state.readBuffer[hostStart] == ' ' || state.readBuffer[hostStart] == '\t'))
            hostStart++;
        size_t hostEnd = state.readBuffer.find("\r\n", hostStart);
        if (hostEnd != std::string::npos) {
            requestHost = state.readBuffer.substr(hostStart, hostEnd - hostStart);
            size_t colonPos = requestHost.find(':');
            if (colonPos != std::string::npos)
                requestHost = requestHost.substr(0, colonPos);
        }
    }

    const ServerConfig* selectedServer = NULL;
    for (size_t i = 0; i < candidates.size(); ++i) {
        const ServerConfig *conf = candidates[i];
        for (size_t j = 0; j < conf->serverNames.size(); ++j) {
            if (conf->serverNames[j] == requestHost) {
                selectedServer = conf;
                break;
            }
        }
        if (selectedServer) break;
    }

    if (!selectedServer && !candidates.empty())
        selectedServer = candidates[0];

    if (!selectedServer) return false;

    _clientToVirtualServer[client_fd] = selectedServer;
    return true;
}

// Validate body size and parse the HTTP request
bool Server::validateAndParseRequest(int client_fd, ClientState &state, HttpRequest &req, const ServerConfig *serverConf)
{
    // Check if complete body is received (Content-Length or chunked)
    size_t hdrEnd = state.readBuffer.find("\r\n\r\n");
    size_t contentLen = 0;
    size_t contentLenPos = state.readBuffer.find("Content-Length:");
    if (contentLenPos != std::string::npos) {
        size_t lineEnd = state.readBuffer.find("\r\n", contentLenPos);
        size_t valStart = contentLenPos + strlen("Content-Length:");
        std::string val = state.readBuffer.substr(valStart, lineEnd - valStart);
        val = ftTrim(val);
        contentLen = static_cast<size_t>(atoi(val.c_str()));
    }

    // std::cout << state.readBuffer.size() << "\n";
    // std::cout << "Content-Length: " << contentLen << "\n";

    if (contentLenPos != std::string::npos) {
        size_t bodyLen = state.readBuffer.size() - (hdrEnd + 4);
        if (bodyLen < contentLen) return false;  // Wait for more data
    }


    // Handle chunked encoding if necessary
    bool isChunked = false;
    size_t tePos = state.readBuffer.find("Transfer-Encoding:");
    if (tePos != std::string::npos) {
        size_t lineEnd = state.readBuffer.find("\r\n", tePos);
        std::string teLine = state.readBuffer.substr(tePos, lineEnd - tePos);
        if (teLine.find("chunked") != std::string::npos) isChunked = true;
    }

    if (isChunked) {
        size_t zeroChunkPos = state.readBuffer.find("\r\n0\r\n", hdrEnd + 4);
        if (zeroChunkPos == std::string::npos) return false;  // Wait for more data
        std::string chunkedBody = state.readBuffer.substr(hdrEnd + 4);
        std::string dechunked = dechunkBody(chunkedBody);
        if (dechunked.empty() && !chunkedBody.empty()) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, NULL));
            state.readBuffer.clear();
            return false;
        }
        if (serverConf->clientMaxBodySize > 0 && dechunked.size() > serverConf->clientMaxBodySize) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *serverConf, NULL));
            state.readBuffer.clear();
            return false;
        }
        std::string headers = state.readBuffer.substr(0, hdrEnd + 4);
        state.readBuffer = headers + dechunked;
    }

    // Parse the request
    HttpRequestParser parser;
    try {
        req = parser.parseRequest(state.readBuffer);
        if (req.httpVersion != "HTTP/1.0" && req.httpVersion != "HTTP/1.1") {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(505, *serverConf, NULL));
            return false;
        }
    } catch (...) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, NULL));
        state.readBuffer.clear();
        return false;
    }

    return true;
}

// Handle DELETE and POST (multipart) requests
bool Server::handleSpecialMethods(int client_fd, const HttpRequest &req, const ServerConfig *serverConf, const LocationConfig *locationConf, const std::string &rawRequest)
{
    if (req.method == "POST") {
        std::string contentType = req.headers.count("Content-Type") ? req.headers.at("Content-Type") : "";
        if (contentType.find("multipart/form-data") != std::string::npos) {
            // Determine upload root
            std::string uploadRoot;
            if (locationConf && !locationConf->uploadDir.empty())
                uploadRoot = locationConf->uploadDir;
            else if (locationConf && !locationConf->root.empty())
                uploadRoot = locationConf->root;
            else
                uploadRoot = serverConf->root;

            std::string uploadDir = uploadRoot;
            if (uploadDir[uploadDir.size() - 1] != '/')
                uploadDir += "/";
            uploadDir += "uploads";

            // Create uploads directory if needed
            struct stat st;
            if (stat(uploadDir.c_str(), &st) == -1) {
                std::string accum;
                size_t pos = 0;
                if (uploadDir.size() > 0 && uploadDir[0] == '/') {
                    accum = "/";
                    pos = 1;
                }
                while (pos <= uploadDir.size()) {
                    size_t next = uploadDir.find('/', pos);
                    if (next == std::string::npos) next = uploadDir.size();
                    std::string part = uploadDir.substr(pos, next - pos);
                    if (!part.empty()) {
                        if (accum.size() > 1 && accum[accum.size() - 1] != '/') accum += "/";
                        accum += part;
                        if (stat(accum.c_str(), &st) == -1) {
                            if (mkdir(accum.c_str(), 0755) == -1 && errno != EEXIST) {
                                std::cerr << "Failed to create upload dir: " << accum << " errno=" << errno << "\n";
                                break;
                            }
                        }
                    }
                    pos = next + 1;
                }
            }

            // Call the multipart handler
            handleMultipartUpload(req, rawRequest, uploadDir, client_fd);

            // Send HTTP 303 redirect
            std::ostringstream response;
            response << "HTTP/1.0 303 See Other\r\n";
            response << "Location: /uploads\r\n";
            response << "Content-Length: 0\r\n";
            response << "Connection: close\r\n\r\n";

            send(client_fd, response.str().c_str(), response.str().size(), 0);
            return true;
        }
    }

    if (req.method == "DELETE") {
        std::string root = (locationConf && !locationConf->root.empty())
                            ? locationConf->root
                            : serverConf->root;

        std::string rawRel;
        if (locationConf && !locationConf->path.empty()
            && req.uri.find(locationConf->path) == 0) {
            rawRel = req.uri.substr(locationConf->path.size());
        } else if (!req.uri.empty() && req.uri[0] == '/') {
            rawRel = req.uri.substr(1);
        } else {
            rawRel = req.uri;
        }

        if (rawRel.empty()) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return true;
        }

        std::string normRel = normalizeRelativePath(rawRel);
        std::string targetPath = root;

        if (locationConf && !locationConf->path.empty() && locationConf->path != "/") {
            std::string locp = locationConf->path;
            if (locp[0] == '/') locp.erase(0, 1);
            targetPath += "/" + locp;
        }

        if (!normRel.empty()) {
            if (normRel[0] != '/')
                targetPath += "/" + normRel;
            else
                targetPath += normRel;
        }

        std::string canonTarget;
        if (!isPathInsideRoot(root, targetPath, canonTarget)) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return true;
        }
        
        struct stat st;
        if (stat(canonTarget.c_str(), &st) == -1) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(404, *serverConf, locationConf));
            return true;
        }

        if (S_ISDIR(st.st_mode)) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return true;
        }

        if (access(canonTarget.c_str(), W_OK) != 0) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return true;
        }

        if (unlink(canonTarget.c_str()) == -1) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(500, *serverConf, locationConf));
            return true;
        }

        std::string resp = "HTTP/1.0 204 No Content\r\n"
                          "Content-Length: 0\r\n"
                          "Connection: close\r\n\r\n";
        queueResponse(client_fd, resp);
        return true;
    }

    return false;  // Not a special method
}

void Server::handleClientData(size_t index)
{
    int client_fd = _fds[index].fd;
    char buffer[BUFFER_SIZE];

    ssize_t received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        if (received < 0) closeClient(index);
        return;
    }

    ClientState &state = _clients[client_fd];
    state.readBuffer.append(buffer, received);
    state.lastActivity = time(NULL);

    // Check body size before parsing
    size_t hdrEnd = state.readBuffer.find("\r\n\r\n");
    if (hdrEnd != std::string::npos) {
        size_t bodyStart = hdrEnd + 4;
        size_t bodyBytes = (state.readBuffer.size() > bodyStart)
                           ? state.readBuffer.size() - bodyStart
                           : 0;
        state.receivedBody = bodyBytes;

        const ServerConfig* serverConfTmp = _clientToVirtualServer[client_fd];
        size_t maxBody = serverConfTmp ? serverConfTmp->clientMaxBodySize : 0;

        if (maxBody > 0 && state.receivedBody > maxBody) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *_clientToVirtualServer[client_fd], NULL));
            return;
        }
    }

    // Select the appropriate server for this client
    if (!_clientToVirtualServer[client_fd]) {
        if (!selectServerForClient(client_fd, state))
            return;
    }

    const ServerConfig *serverConf = _clientToVirtualServer[client_fd];
    if (!serverConf) return;

    // Parse the HTTP request
    HttpRequest req;
    if (!validateAndParseRequest(client_fd, state, req, serverConf))
        return;

    // Find the best matching location
    const LocationConfig* locationConf = NULL;
    size_t bestMatchLen = 0;
    for (size_t i = 0; i < serverConf->locations.size(); ++i) {
        const LocationConfig &loc = serverConf->locations[i];
        if (req.uri.find(loc.path) == 0 && loc.path.size() > bestMatchLen) {
            locationConf = &loc;
            bestMatchLen = loc.path.size();
        }
    }

    // Fallback if no specific location found
    if (!locationConf && !serverConf->locations.empty())
        locationConf = &serverConf->locations[0];

    // Check client max body size

    std::cout << "Client Max Body Size: " << serverConf->clientMaxBodySize << "\n";
    std::cout << "Request Content Length: " << req.contentLength << "\n";

    if (checkClientMaxBodySize(req.contentLength, serverConf->clientMaxBodySize)) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *serverConf, locationConf));
        return;
    }

    std::cout << "\n\n*****************\n\n";

    // Check allowed HTTP methods
    std::set<std::string> allowed;
    if (locationConf && !locationConf->methods.empty()) {
        allowed.insert(locationConf->methods.begin(), locationConf->methods.end());
    }

    if (req.method.empty() || (req.method != "GET" && req.method != "POST" && req.method != "DELETE")) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, locationConf));
        return;
    }
    else if (allowed.find(req.method) == allowed.end()) {
        std::string allowHeader;
        if (!allowed.empty()) {
            allowHeader = "Allow: ";
            bool first = true;
            for (std::set<std::string>::const_iterator it = allowed.begin(); it != allowed.end(); ++it) {
                if (!first) allowHeader += ", ";
                allowHeader += *it;
                first = false;
            }
            allowHeader += "\r\n";
        }
        queueResponse(client_fd, HandleErrors::generateErrorResponse(405, *serverConf, locationConf, allowHeader));
        return;
    }

    // Handle special methods (POST multipart, DELETE)
    if (handleSpecialMethods(client_fd, req, serverConf, locationConf, state.readBuffer))
        return;

    // Handle normal GET requests (and other methods)
    HttpResponseBuilder builder(_mimeTypes);
    
    // Check if this is a CGI request - handle asynchronously
    std::string cgiScript;
    if (locationConf && isCgiRequest(req, *locationConf, cgiScript))
    {
        // Create a copy of locationConf to ensure stability across async handling
        LocationConfig locCopy = *locationConf;
        
        // Don't execute CGI synchronously - queue it for async handling
        HandleCGI cgi(req, *serverConf, locCopy);
        cgi.buildEnv();
        CGIProcess *cgiProc = cgi.execute();
        
        if (cgiProc && cgiProc->pid > 0) {
            // Store the CGI process for tracking
            _cgiProcesses[client_fd] = cgiProc;
            _pidToClientFd[cgiProc->pid] = client_fd;
            std::cout << "Started async CGI process " << cgiProc->pid << " for client " << client_fd << "\n";
            return;  // Don't send response yet, it will be sent when CGI finishes
        } else {
            std::string response = HandleErrors::generateErrorResponse(500, *serverConf, locationConf);
            queueResponse(client_fd, response);
            return;
        }
    }
    
    // Non-CGI requests handled synchronously
    std::string response;
    try {
        response = builder.buildResponse(req, *serverConf, locationConf ? *locationConf : LocationConfig());
    } catch (std::exception &e) {
        response = HandleErrors::generateErrorResponse(500, *serverConf, locationConf);
    }

    queueResponse(client_fd, response);
}


void Server::run()
{
    ServerConfig defaultServerConf;

    extern volatile sig_atomic_t g_stop;

    while (!g_stop)
    {
        int poll_count = poll(_fds.data(), _fds.size(), 1000);
        if (poll_count == -1)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "Error: poll failed\n";
            break;
        }

        pollCGIProcesses();

        for (int k = static_cast<int>(_fds.size()) - 1; k >= 0; --k)
        {
            size_t i = static_cast<size_t>(k);
            if (_fds[i].revents & POLLIN)
            {
                if (_listenSockets.count(_fds[i].fd)) {
                    handleNewConnection(i);
                }
                else {
                    handleClientData(i);
                }
            }
            if (_fds[i].revents & POLLOUT) {
                handlePollOut(i);
            }
            if (_fds[i].revents & (POLLHUP | POLLERR)) {
                closeClient(i);
            }
        }

        time_t now = time(NULL);
        for (int k = static_cast<int>(_fds.size()) - 1; k >= 0; --k) {
            size_t i = static_cast<size_t>(k);
            int fd = _fds[i].fd;
            if (_listenSockets.count(fd)) continue;
            if (_clients.find(fd) == _clients.end()) continue;
            ClientState &st = _clients[fd];
            if (st.lastActivity != 0 && now - st.lastActivity > CLIENT_IDLE_TIMEOUT_SEC) {
                std::cout << "Closing idle client " << fd << " after " << (now - st.lastActivity) << "s\n";
                std::string resp = HandleErrors::generateErrorResponse(408, defaultServerConf, NULL);
                queueResponse(fd, resp);
                _closeAfterSend[fd] = true;
                continue;
            }
            if (st.createdAt != 0 && now - st.createdAt > CLIENT_TOTAL_TIMEOUT_SEC) {
                std::cout << "Closing client " << fd << " due to total timeout\n";
                std::string resp = HandleErrors::generateErrorResponse(408, defaultServerConf, NULL);
                queueResponse(fd, resp);
                _closeAfterSend[fd] = true;
                continue;
            }
        }
    }
}


void Server::queueResponse(int client_fd, const std::string &response)
{
    if (client_fd < 0)
        return;

    _sendBuffers[client_fd] = response;
    _closeAfterSend[client_fd] = true; 

    for (size_t i = 0; i < _fds.size(); ++i) {
        if (_fds[i].fd == client_fd) {
            _fds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::handlePollOut(size_t index)
{
    int fd = _fds[index].fd;
    if (_sendBuffers.find(fd) == _sendBuffers.end()) {
        // nothing to send, disable POLLOUT
        _fds[index].events &= ~POLLOUT;
        return;
    }
    std::string &buf = _sendBuffers[fd];
    const char *data = buf.c_str();
    size_t toSend = buf.size();
    ssize_t n = ::send(fd, data, toSend, 0);
    if (n <= 0) {
        // cannot send now or fatal -> close client (server uses poll to retry)
        // update activity? close
        closeClient(index);
        return;
    }
    if ((size_t)n >= buf.size()) {
        // all sent
        _sendBuffers.erase(fd);
        _fds[index].events &= ~POLLOUT;
        if (_closeAfterSend[fd]) {
            _closeAfterSend.erase(fd);
            closeClient(index);
        }
    } else {
        // partial sent, remove sent portion
        buf.erase(0, n);
        // update activity timestamp on successful send
        if (_clients.find(fd) != _clients.end())
            _clients[fd].lastActivity = time(NULL);
        // keep POLLOUT to continue sending on next poll
    }
}

void Server::closeClient(size_t index)
{
    if (index >= _fds.size()) return;
    int fd = _fds[index].fd;
    ::close(fd);
    // remove from client sockets vector if present
    for (std::vector<int>::iterator it = _clientSockets.begin(); it != _clientSockets.end(); ++it) {
        if (*it == fd) { _clientSockets.erase(it); break; }
    }
    _clientToVirtualServer.erase(fd);
    _clientToListenSocket.erase(fd);
    _clients.erase(fd);
    _sendBuffers.erase(fd);
    _closeAfterSend.erase(fd);
    // remove the pollfd entry
    _fds.erase(_fds.begin() + index);
}

void Server::cleanup()
{
    std::cout << "Closing all client connections...\n";
    
    while (!_clientSockets.empty())
    {
        closeClient(0);
    }

    std::cout << "Closing listening sockets...\n";
    // Closing listening sockets
    for (size_t i = 0; i < _fds.size(); ++i)
        ::close(_fds[i].fd);
    _fds.clear();
    _listenSockets.clear();

    std::cout << "Server cleanup done.\n";
}

// Poll CGI processes to check if they've completed
void Server::pollCGIProcesses()
{
    std::vector<int> completedClients;
    
    for (std::map<int, CGIProcess*>::iterator it = _cgiProcesses.begin(); 
         it != _cgiProcesses.end(); ++it) {
        int client_fd = it->first;
        CGIProcess *cgiProc = it->second;
        
        if (!cgiProc) continue;
        
        time_t now = time(NULL);
        long elapsed = (now - cgiProc->startTime) * 1000;
        
        // Check timeout
        if (elapsed > cgiProc->timeoutMs) {
            std::cerr << "CGI timeout for client " << client_fd << " (pid " << cgiProc->pid << ")\n";
            cgiProc->timedOut = true;
            kill(cgiProc->pid, SIGKILL);
            completedClients.push_back(client_fd);
            continue;
        }
        
        // Try to read available data
        HandleCGI::readCGIOutput(cgiProc);
        
        // Check if process is still running
        if (cgiProc->pid > 0) {
            int status = 0;
            pid_t result = waitpid(cgiProc->pid, &status, WNOHANG);
            
            if (result == cgiProc->pid) {
                // Process has finished
                std::cout << "CGI process " << cgiProc->pid << " finished for client " << client_fd << "\n";
                
                // Try to read any remaining data
                HandleCGI::readCGIOutput(cgiProc);
                
                // Mark as completed
                completedClients.push_back(client_fd);
            } else if (result < 0) {
                // ECHILD means process was already reaped by SIGCHLD handler (SIG_IGN)
                // This is normal with signal(SIGCHLD, SIG_IGN) - just mark as completed
                if (errno == ECHILD) {
                    std::cout << "CGI process " << cgiProc->pid << " already reaped (normal)\n";
                } else {
                    std::cerr << "waitpid error for CGI process: " << strerror(errno) << "\n";
                }
                completedClients.push_back(client_fd);
            }
        }
    }
    
    // Process completed CGI requests
    for (size_t i = 0; i < completedClients.size(); ++i) {
        finalizeCGI(completedClients[i], _cgiProcesses[completedClients[i]]);
    }
}

// Finalize CGI: send response to client
void Server::finalizeCGI(int client_fd, CGIProcess *cgiProc)
{
    if (!cgiProc) return;
    
    // Close pipes
    if (cgiProc->pipe_out >= 0) close(cgiProc->pipe_out);
    if (cgiProc->pipe_err >= 0) close(cgiProc->pipe_err);
    
    // Get the server configuration for this client
    const ServerConfig *serverConf = _clientToVirtualServer[client_fd];
    if (!serverConf) {
        // Fallback to first server if somehow not found
        serverConf = &_config.servers[0];
    }
    
    std::string response;
    
    // Check if CGI timed out
    if (cgiProc->timedOut) {
        response = HandleErrors::generateErrorResponse(408, *serverConf, NULL);
    }
    // Check for errors
    else if (!cgiProc->error.empty()) {
        std::cerr << "CGI stderr: " << cgiProc->error << "\n";
        response = HandleErrors::generateErrorResponse(500, *serverConf, NULL);
    } else {
        // Process CGI output
        std::string &output = cgiProc->output;
        size_t pos = output.find("\r\n\r\n");
        if (pos != std::string::npos) {
            // Parse headers and body
            std::string headers = output.substr(0, pos);
            std::string body = output.substr(pos + 4);
            
            // Build HTTP response
            std::ostringstream resp;
            
            // Extract Status header if present
            std::string statusLine = "HTTP/1.0 200 OK\r\n";
            size_t statusPos = headers.find("Status:");
            if (statusPos != std::string::npos) {
                size_t lineEnd = headers.find("\r\n", statusPos);
                std::string statusHeader = headers.substr(statusPos + 7);
                if (lineEnd != std::string::npos)
                    statusHeader = statusHeader.substr(0, lineEnd - (statusPos + 7));
                statusLine = std::string("HTTP/1.0 ") + statusHeader + "\r\n";
            }
            
            resp << statusLine;
            
            // Add other headers
            std::istringstream hh(headers);
            std::string line;
            while (std::getline(hh, line)) {
                if (!line.empty() && line[line.size()-1] == '\r') line.erase(line.size()-1);
                if (line.substr(0, 7) != "Status:" && !line.empty()) {
                    resp << line << "\r\n";
                }
            }
            
            // Add Content-Length
            resp << "Content-Length: " << body.size() << "\r\n";
            resp << "Connection: close\r\n\r\n";
            resp << body;
            
            response = resp.str();
        } else {
            response = output;
        }
    }
    
    // Send response to client
    queueResponse(client_fd, response);
    
    // Clean up
    if (cgiProc->pid > 0) {
        _pidToClientFd.erase(cgiProc->pid);
    }
    delete cgiProc;
    _cgiProcesses.erase(client_fd);
}


