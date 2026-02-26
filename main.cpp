#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 4096

int main(int, char **)
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port: " << PORT << std::endl;

    while (true)
    {
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&client_address, &client_address_len);
        if (clientSocket == -1)
        {
            std::cerr << "Faile to accept connection" << std::endl;
            continue;
        }
        std::cout << "Accepted connection from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;

        char buffer[BUFFER_SIZE];
        std::memset(buffer, 0, BUFFER_SIZE);
        std::size_t bytes_received = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1)
        {
            std::cerr << "Failed to receive request" << std::endl;
            closesocket(clientSocket);
            continue;
        }

        std::string request(buffer, bytes_received);
        std::size_t first_space = request.find(' ');
        std::string method = request.substr(0, first_space);
        std::size_t second_space = request.find(' ', first_space + 1);
        std::string path = request.substr(first_space + 1, second_space - first_space - 1);

        std::cout << "Received" << method << " request for " << path << std::endl;
        if (method == "GET")
        {
            if (path == "/test")
            {
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>This is a test </h1></body></html>";
                send(clientSocket, response.c_str(), response.length(), 0);
            }
            else
            {

                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>HELLO</h1></body></html>";
                send(clientSocket, response.c_str(), response.length(), 0);
            }
        }
        else
        {
            std::string response = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
