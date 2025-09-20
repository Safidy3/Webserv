/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:37:39 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:53:19 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "utils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <map>
#include <string>

static const size_t BUFFER_SIZE = 1024;
static const int MAX_PENDING_QUEUE = 10;
static const int MAX_CLIENTS = 100;

class Server
{
    private:
        HttpConfig _config;
        std::vector<struct pollfd> _fds;
        std::vector<int> _clientSockets;
        MimeTypes &_mimeTypes;

        void setupListeningSockets();
        void handleNewConnection(size_t index);
        void handleClientData(size_t index);

    public:
        Server(const HttpConfig &config, MimeTypes &types);
        ~Server();


        void run();
};

#endif
