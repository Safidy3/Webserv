// web_server.cpp
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

int main()
{
    int					server_fd, client_fd;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
	{
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // 2. Bind address
    struct sockaddr_in	server_addr;
	
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    server_addr.sin_port = htons(8080);       // port 8080

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
        std::cerr << "Bind failed\n";
        return 1;
    }

    // 3. Listen
    if (listen(server_fd, 1) < 0)
	{
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server running on port 8080...\n";

    // 4. Accept connection
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
	{
        std::cerr << "Accept failed\n";
        return 1;
    }

    // 5. Read HTTP request
    char				buffer[1024];

    memset(buffer, 0, sizeof(buffer));
    read(client_fd, buffer, sizeof(buffer) - 1);
    std::cout << "Request:\n" << buffer << std::endl;

    // 6. Send HTTP response
    const char* http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";

    write(client_fd, http_response, strlen(http_response));

    // 7. Clean up
    close(client_fd);
    close(server_fd);

    return 0;
}
