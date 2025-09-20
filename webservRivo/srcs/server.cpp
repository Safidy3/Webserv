/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:42:37 by safandri         ###   ########.fr       */
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

        std::cout << "New client: " << client_sock << std::endl;
    }
}

void Server::handleClientData(size_t index)
{
    char buffer[BUFFER_SIZE] = {0};
    int received = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        close(_fds[index].fd);
        _fds.erase(_fds.begin() + index);
        return;
    }

    std::string rawRequest(buffer, received);
    HttpRequestParser parser;
    HttpRequest req = parser.parseRequest(rawRequest);

    const ServerConfig_t &serverConf = _config.servers[0];
    
    const LocationConfig_t* locationConf = NULL;
    for (size_t i = 0; i < serverConf.locations.size(); ++i)
    {
        const LocationConfig_t &loc = serverConf.locations[i];
        if (req.uri.find(loc.path) == 0)  // URI commence par loc.path
        {
            locationConf = &loc;
            break;
        }
    }
    
    // fallback si aucune location ne correspond
    if (!locationConf && !serverConf.locations.empty())
    locationConf = &serverConf.locations[0];
    
    HttpResponseBuilder builder(_mimeTypes);

    std::string response = builder.buildResponse(req, serverConf, *locationConf);
    send(_fds[index].fd, response.c_str(), response.size(), 0);
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
                if (i < _config.servers.size())
                {
                    // C’est un socket d’écoute
                    handleNewConnection(i);
                }
                else
                {
                    // C’est un client
                    handleClientData(i);
                }
            }
        }
    }
}
