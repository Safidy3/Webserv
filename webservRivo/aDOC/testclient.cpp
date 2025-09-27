#include <fstream>
#include <sstream>

void Server::handleClientData(size_t index)
{
    char buffer[BUFFER_SIZE] = {0};
    int received = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0)
    {
        close(_fds[index].fd);
        _fds.erase(_fds.begin() + index);
        return;
    }

    std::string rawRequest(buffer, received);
    HttpRequestParser parser;
    HttpRequest req = parser.parseRequest(rawRequest);

    std::cout << "HTTP Request: " << req.method 
              << " " << req.uri << " " << req.httpVersion << "\n";

    // On prend la config du serveur correspondant
    size_t serverIndex = 0;
    for (; serverIndex < _config.servers.size(); ++serverIndex) {
        if (_fds[index].fd == _fds[serverIndex].fd)
            break;
    }
    const ServerConfig &serverConf = _config.servers[serverIndex];

    // Vérifier la taille du body
    if (req.contentLength > serverConf.clientMaxBodySize) {
        std::string resp = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\n\r\n";
        send(_fds[index].fd, resp.c_str(), resp.size(), 0);
        close(_fds[index].fd);
        _fds.erase(_fds.begin() + index);
        return;
    }

    // Déterminer le fichier à servir
    std::string filePath;
    if (req.uri == "/") {
        // Chercher le premier indexFile existant
        for (size_t i = 0; i < serverConf.indexFiles.size(); ++i) {
            std::string idxFile = serverConf.root + "/" + serverConf.indexFiles[i];
            std::ifstream f(idxFile.c_str());
            if (f.good()) {
                filePath = idxFile;
                break;
            }
        }
    } else {
        filePath = serverConf.root + req.uri;
    }

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    std::string responseBody;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf();
        responseBody = ss.str();
    } else {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        responseBody = "<h1>404 Not Found</h1>";
    }

    std::ostringstream response;
    response << statusLine
             << "Content-Length: " << responseBody.size() << "\r\n"
             << "Content-Type: text/html\r\n\r\n"
             << responseBody;

    std::string respStr = response.str();
    send(_fds[index].fd, respStr.c_str(), respStr.size(), 0);
}
