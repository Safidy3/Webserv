/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/11 17:51:46 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpServer.hpp"
#include "../include/httpResponse.hpp"
#include "../include/httpConfig.hpp"
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "../include/utils.hpp"
#include <fcntl.h>

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
    // Grouper les serveurs par (host, port) pour créer UNE socket par endpoint
    std::map<std::pair<std::string, int>, std::vector<size_t> > serversByEndpoint;
    for (size_t i = 0; i < _config.servers.size(); ++i) {
        std::pair<std::string, int> endpoint(_config.servers[i].host, _config.servers[i].listenPort);
        serversByEndpoint[endpoint].push_back(i);
    }
    
    // Pour chaque endpoint unique, créer une socket et y ajouter tous les serveurs
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
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(endpoint.second);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            std::cerr << "Error: bind failed on port " << endpoint.second
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

        // Ajouter TOUS les serveurs de cet endpoint
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
    // set client socket non-blocking
    int flags = fcntl(client_sock, F_GETFL, 0);
    fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
    struct pollfd client_pfd;
    client_pfd.fd = client_sock;
    client_pfd.events = POLLIN;
    client_pfd.revents = 0;
    _fds.push_back(client_pfd);
        _clientSockets.push_back(client_sock);
    // initialize client parsing state
        _clients[client_sock] = ClientState();
        time_t now = time(NULL);
        _clients[client_sock].lastActivity = now;
        _clients[client_sock].createdAt = now;

        // Stocker le socket d'écoute pour choisir le serveur plus tard
        // On va stocker l'index du fd d'écoute dans une map temporaire
        // pour retrouver la liste des serveurs possibles
        if (_listenSockets.count(_fds[index].fd)) {
            // On ne choisit pas le serveur maintenant
            // On laissera handleClientData() le faire selon le header Host:
            _clientToServer[client_sock] = NULL;  // À déterminer plus tard
            _clientToListenSocket[client_sock] = _fds[index].fd;  // Stocker le listen fd
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
    std::string body = rawRequest.substr(rawRequest.find("\r\n\r\n") + 4);
    std::string delimiter = "--" + boundary;

    std::vector<std::string> parts = splitParts(body, delimiter);

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
        "HTTP/1.1 201 Created\r\n"
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

    //* BODY CHECKING BEFORE PARSING
    size_t hdrEnd = state.readBuffer.find("\r\n\r\n");
    if (hdrEnd != std::string::npos) {
        size_t bodyStart = hdrEnd + 4;
        size_t bodyBytes = (state.readBuffer.size() > bodyStart)
                           ? state.readBuffer.size() - bodyStart
                           : 0;
        state.receivedBody = bodyBytes;

        const ServerConfig* serverConfTmp = _clientToServer[client_fd];
        size_t maxBody = serverConfTmp ? serverConfTmp->clientMaxBodySize : 0;

        if (maxBody > 0 && state.receivedBody > maxBody) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *_clientToServer[client_fd], NULL));
            return;
        }
    }

    //* Check if assigned server
    if (!_clientToServer[client_fd]) {
        if (_clientToListenSocket.count(client_fd) == 0)
            return;

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
        std::string matchedName;
        for (size_t i = 0; i < candidates.size(); ++i) {
            const ServerConfig *conf = candidates[i];
            for (size_t j = 0; j < conf->serverNames.size(); ++j) {
                if (conf->serverNames[j] == requestHost) {
                    selectedServer = conf;
                    matchedName = conf->serverNames[j];
                    break;
                }
            }
            if (selectedServer) break;
        }

        if (!selectedServer && !candidates.empty())
            selectedServer = candidates[0];

        if (!selectedServer) return;

        _clientToServer[client_fd] = selectedServer;
    }

    const ServerConfig *serverConf = _clientToServer[client_fd];
    if (!serverConf) return;

    // Vérifier si le body complet est reçu (Content-Length ou chunked)
    size_t contentLen = 0;
    size_t contentLenPos = state.readBuffer.find("Content-Length:");
    if (contentLenPos != std::string::npos) {
        size_t lineEnd = state.readBuffer.find("\r\n", contentLenPos);
        size_t valStart = contentLenPos + strlen("Content-Length:");
        std::string val = state.readBuffer.substr(valStart, lineEnd - valStart);
        val = ftTrim(val);
        contentLen = static_cast<size_t>(atoi(val.c_str()));
    }

    if (contentLenPos != std::string::npos) {
        size_t bodyLen = state.readBuffer.size() - (hdrEnd + 4);
        if (bodyLen < contentLen) return; // attendre le reste
    }

    // gérer chunked si nécessaire
    bool isChunked = false;
    size_t tePos = state.readBuffer.find("Transfer-Encoding:");
    if (tePos != std::string::npos) {
        size_t lineEnd = state.readBuffer.find("\r\n", tePos);
        std::string teLine = state.readBuffer.substr(tePos, lineEnd - tePos);
        if (teLine.find("chunked") != std::string::npos) isChunked = true;
    }

    if (isChunked) {
        size_t zeroChunkPos = state.readBuffer.find("\r\n0\r\n", hdrEnd + 4);
        if (zeroChunkPos == std::string::npos) return; // attendre le reste
        std::string chunkedBody = state.readBuffer.substr(hdrEnd + 4);
        std::string dechunked = dechunkBody(chunkedBody);
        if (dechunked.empty() && !chunkedBody.empty()) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, NULL));
            state.readBuffer.clear();
            return;
        }
        if (serverConf->clientMaxBodySize > 0 && dechunked.size() > serverConf->clientMaxBodySize) {
            queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *serverConf, NULL));
            state.readBuffer.clear();
            return;
        }
        std::string headers = state.readBuffer.substr(0, hdrEnd + 4);
        state.readBuffer = headers + dechunked;
    }

    // On peut maintenant parser la requête
    HttpRequestParser parser;
    HttpRequest req;
    try {
        req = parser.parseRequest(state.readBuffer);
    } catch (...) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, NULL));
        state.readBuffer.clear();
        return;
    }

    // 🔹 Trouver la meilleure location
    const LocationConfig* locationConf = NULL;
    size_t bestMatchLen = 0;
    for (size_t i = 0; i < serverConf->locations.size(); ++i) {
        const LocationConfig &loc = serverConf->locations[i];
        if (req.uri.find(loc.path) == 0 && loc.path.size() > bestMatchLen) {
            locationConf = &loc;
            bestMatchLen = loc.path.size();
        }
    }

    // fallback si aucune location spécifique trouvée
    if (!locationConf && !serverConf->locations.empty())
        locationConf = &serverConf->locations[0];

    // Enforcer client_max_body_size (après détermination de la location)
    if (checkClientMaxBodySize(req.contentLength, serverConf->clientMaxBodySize)) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(413, *serverConf, locationConf));
        return;
    }

    // 🔹 Vérifier méthode autorisée (avec fallback par défaut)
    std::set<std::string> allowed;
    if (locationConf && !locationConf->methods.empty()) {
        allowed.insert(locationConf->methods.begin(), locationConf->methods.end());
    } else {
        allowed.insert("GET");
        allowed.insert("POST");
        allowed.insert("DELETE");
    }

    // Vérifier que la méthode est valide (non vide)
    if (req.method.empty()) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(400, *serverConf, locationConf));
        return;
    }
    
    if (allowed.find(req.method) == allowed.end()) {
        queueResponse(client_fd, HandleErrors::generateErrorResponse(405, *serverConf, locationConf, "Allow: GET, POST, DELETE\r\n"));
        return;
    }

    if (req.method == "POST") {
        std::string contentType = req.headers["Content-Type"];
        if (contentType.find("multipart/form-data") != std::string::npos) {

            // Déterminer la racine d'upload
            std::string uploadRoot;
            if (locationConf && !locationConf->uploadDir.empty())
                uploadRoot = locationConf->uploadDir;
            else if (locationConf && !locationConf->root.empty())
                uploadRoot = locationConf->root;
            else
                uploadRoot = serverConf->root;

            // Ajouter "/uploads" pour tous les cas
            std::string uploadDir = uploadRoot;
            if (uploadDir[uploadDir.size() - 1] != '/')
                uploadDir += "/";
            uploadDir += "uploads";

            // Créer le dossier /uploads si nécessaire
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

            // Appeler le handler multipart avec le bon chemin
            handleMultipartUpload(req, state.readBuffer, uploadDir, client_fd);

            // Ensuite, envoyer une redirection HTTP 303
            std::ostringstream response;
            response << "HTTP/1.1 303 See Other\r\n";
            response << "Location: /uploads\r\n"; // ou /uploads/success.html si tu préfères
            response << "Content-Length: 0\r\n";
            response << "Connection: close\r\n\r\n";

            send(client_fd, response.str().c_str(), response.str().size(), 0);
            return;
        }
    }

    if (req.method == "DELETE")
    {
        std::string root = (locationConf && !locationConf->root.empty())
                            ? locationConf->root
                            : serverConf->root;

        std::string rawRel;
        if (locationConf && !locationConf->path.empty()
            && req.uri.find(locationConf->path) == 0)
        {
            rawRel = req.uri.substr(locationConf->path.size());
        }
        else if (!req.uri.empty() && req.uri[0] == '/')
            rawRel = req.uri.substr(1);
        else
            rawRel = req.uri;

        if (rawRel.empty()) {
            queueResponse(client_fd,
                HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return;
        }

        std::string normRel = normalizeRelativePath(rawRel);

        std::string targetPath = root;

        if (locationConf && !locationConf->path.empty()
            && locationConf->path != "/")
        {
            std::string locp = locationConf->path;
            if (locp[0] == '/') locp.erase(0,1);
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
            queueResponse(client_fd,
                HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return;
        }
        
        struct stat st;
        if (stat(canonTarget.c_str(), &st) == -1) {
            queueResponse(client_fd,
                HandleErrors::generateErrorResponse(404, *serverConf, locationConf));
            return;
        }

        if (S_ISDIR(st.st_mode)) {
            queueResponse(client_fd,
                HandleErrors::generateErrorResponse(403, *serverConf, locationConf));
            return;
        }

        if (unlink(canonTarget.c_str()) == -1) {
            queueResponse(client_fd,
                HandleErrors::generateErrorResponse(500, *serverConf, locationConf));
            return;
        }

        std::string resp =
            "HTTP/1.1 204 No Content\r\n"
            "Content-Length: 0\r\n"
                "Connection: close\r\n\r\n";

            queueResponse(client_fd, resp);
            return;
    }

    HttpResponseBuilder builder(_mimeTypes);
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
    while (true)
    {
        int poll_count = poll(_fds.data(), _fds.size(), 1000); // 1s timeout to allow timeouts handling
        if (poll_count == -1)
        {
            std::cerr << "Error: poll failed\n";
            break;
        }
        // Iterate backwards so that removing an fd doesn't affect unprocessed indices
        for (int ii = static_cast<int>(_fds.size()) - 1; ii >= 0; --ii)
        {
            size_t i = static_cast<size_t>(ii);
            if (_fds[i].revents & POLLIN)
            {
                if (_listenSockets.count(_fds[i].fd)) {
                    // C’est un socket d’écoute
                    handleNewConnection(i);
                }
                else {
                    // C’est un client
                    handleClientData(i);
                }
            }
            if (_fds[i].revents & POLLOUT) {
                // handle pending send
                handlePollOut(i);
            }
            if (_fds[i].revents & (POLLHUP | POLLERR)) {
                // close client and cleanup
                closeClient(i);
            }
        }

        // Timeout sweep: close idle or too-old connections
        time_t now = time(NULL);
        for (int ii = static_cast<int>(_fds.size()) - 1; ii >= 0; --ii) {
            size_t i = static_cast<size_t>(ii);
            int fd = _fds[i].fd;
            // skip listening sockets
            if (_listenSockets.count(fd)) continue;
            if (_clients.find(fd) == _clients.end()) continue;
            ClientState &st = _clients[fd];
            if (st.lastActivity != 0 && now - st.lastActivity > CLIENT_IDLE_TIMEOUT_SEC) {
                // idle timeout
                std::cout << "Closing idle client " << fd << " after " << (now - st.lastActivity) << "s\n";
                closeClient(i);
                continue;
            }
            if (st.createdAt != 0 && now - st.createdAt > CLIENT_TOTAL_TIMEOUT_SEC) {
                std::cout << "Closing client " << fd << " due to total timeout\n";
                closeClient(i);
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
    _clientToServer.erase(fd);
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
    // Fermer les sockets d'écoute
    for (size_t i = 0; i < _fds.size(); ++i)
        ::close(_fds[i].fd);
    _fds.clear();
    _listenSockets.clear();

    std::cout << "Server cleanup done.\n";
}



