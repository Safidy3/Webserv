/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/24 16:19:16 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpServer.hpp"
#include "../include/httpResponse.hpp"


Server::Server(const HttpConfig &config, MimeTypes &types) : _config(config), _mimeTypes(types)
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
    for (size_t i = 0; i < _config.servers.size(); ++i)
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            std::cerr << "Error: socket init failed\n";
            continue;
        }

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY; // TODO: convertir host string en in_addr
        addr.sin_port = htons(_config.servers[i].listenPort);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
        {
            std::cerr << "Error: bind failed on port " << _config.servers[i].listenPort << "\n";
            close(sock);
            continue;
        }

        if (listen(sock, MAX_PENDING_QUEUE) == -1)
        {
            std::cerr << "Error: listen failed on port " << _config.servers[i].listenPort << "\n";
            close(sock);
            continue;
        }

        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        _fds.push_back(pfd);

        // 👇 Ici : association socket d'écoute ↔ config serveur
        _listenSockets[sock] = &_config.servers[i];

        std::cout << "Listening on " << _config.servers[i].host 
                << ":" << _config.servers[i].listenPort << std::endl;
    }
}

void Server::handleNewConnection(size_t index)
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_sock = accept(_fds[index].fd, (sockaddr*)&client_addr, &addrlen);
    if (client_sock != -1)
    {
        struct pollfd client_pfd;
        client_pfd.fd = client_sock;
        client_pfd.events = POLLIN;
        _fds.push_back(client_pfd);
        _clientSockets.push_back(client_sock);

        // Vérifie que le fd d’écoute est bien dans la map
        if (_listenSockets.count(_fds[index].fd)) {
            _clientToServer[client_sock] = _listenSockets[_fds[index].fd];
            std::cout << "New client " << client_sock 
                      << " attached to server listening on port "
                      << _listenSockets[_fds[index].fd]->listenPort << std::endl;
        } else {
            std::cerr << "Error: listening socket " << _fds[index].fd 
                      << " not found in _listenSockets!" << std::endl;
        }
    }
}

void Server::handleClientData(size_t index)
{
    char buffer[BUFFER_SIZE] = {0};
    int client_fd = _fds[index].fd;

    int received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        close(client_fd);
        _clientToServer.erase(client_fd);
        _fds.erase(_fds.begin() + index);
        return;
    }

    std::string rawRequest(buffer, received);
    HttpRequestParser parser;
    HttpRequest req;

    try {
        req = parser.parseRequest(rawRequest);
    } catch (std::exception &e) {
        // 🔹 Requête malformée → 400 Bad Request
        std::string response = HandleErrors::generateErrorResponse(
            400, *_clientToServer[client_fd], NULL
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }
    // 🔹 Récupérer la config du serveur associé
    ServerConfig *serverConf = _clientToServer[client_fd];
    if (!serverConf) {
        std::cerr << "Error: no server config found for client " << client_fd << "\n";
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

    // 🔹 Vérifier méthode autorisée (avec fallback par défaut)
    std::set<std::string> allowed;
    if (locationConf && !locationConf->methods.empty()) {
        allowed.insert(locationConf->methods.begin(), locationConf->methods.end());
    } else {
        allowed.insert("GET");
        allowed.insert("POST");
        allowed.insert("DELETE");
    }

    if (allowed.find(req.method) == allowed.end()) {
        std::string response = HandleErrors::generateErrorResponse(
            405, *serverConf, locationConf, "Allow: GET, POST, DELETE\r\n"
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    // 🔹 Construire et envoyer la réponse HTTP
    HttpResponseBuilder builder(_mimeTypes);
    std::string response;
    try {
        response = builder.buildResponse(req, *serverConf, 
                                         locationConf ? *locationConf : LocationConfig());
    } catch (std::exception &e) {
        response = HandleErrors::generateErrorResponse(500, *serverConf, locationConf);
    }

    send(client_fd, response.c_str(), response.size(), 0);
}


void Server::run()
{
    while (true)
    {
        int poll_count = poll(_fds.data(), _fds.size(), -1);
        if (poll_count == -1)
        {
            std::cerr << "Error: poll failed\n";
            break;
        }

        for (size_t i = 0; i < _fds.size(); ++i)
        {
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
        }

    }
}

