#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <mimetype_utils.h>
#define PORT 8080
#define BUFFER_SIZE 4096
#define WORKING_DIR "E:\\CODES\\Practice-http\\src"

std::string urlDecode(const std::string &str)
{
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.size())
        {
            std::string hex = str.substr(i + 1, 2);
            char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += decoded_char;
            i += 2;
        }
        else if (str[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

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

        std::string request;
        char buffer[BUFFER_SIZE];

        while (true)
        {
            int n = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (n <= 0)
                break;
            request.append(buffer, n);
            if (request.find("\r\n\r\n") != std::string::npos)
                break;
        }

        std::size_t first_space = request.find(' ');
        std::string method = request.substr(0, first_space);
        size_t second_space = request.find(' ', first_space + 1);

        std::string target = request.substr(first_space + 1, second_space - first_space - 1);
        std::string path;
        std::string query;

        size_t query_pos = target.find('?');

        if (query_pos != std::string::npos)
        {
            path = target.substr(0, query_pos);
            query = target.substr(query_pos + 1);
        }
        else
        {
            path = target;
        }
        path = urlDecode(path);

        std::cout << "Received" << method << " request for " << path << std::endl;
        if (method == "GET")
        {
            if (path == "/favicon.ico")
            {
                std::string resp =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n\r\n";
                send(clientSocket, resp.c_str(), (int)resp.size(), 0);
                closesocket(clientSocket);
                continue;
            }
            if (path == "/test")
            {
                std::string body = "<html><body><img src='./cheval.png'/></body></html>";
                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=UTF-8\r\n"
                    "Content-Length: " +
                    std::to_string(body.size()) + "\r\n"
                                                  "Connection: close\r\n"
                                                  "\r\n" +
                    body;

                send(clientSocket, response.c_str(), (int)response.size(), 0);
            }
            else if (path == "/")
            {
                std::string body = "<html><body><h1>HELLO</h1></body></html>";

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=UTF-8\r\n"
                    "Content-Length: " +
                    std::to_string(body.size()) + "\r\n"
                                                  "Connection: close\r\n"
                                                  "\r\n" +
                    body;

                send(clientSocket, response.c_str(), (int)response.size(), 0);
            }
            else
            {
                std::filesystem::path filePath = WORKING_DIR + path;
                if (!std::filesystem::exists(filePath))
                {
                    std::string response =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                    
                    send(clientSocket, response.c_str(), (int)response.size(), 0);
                    continue;
                }

                std::string ext = filePath.extension().string();
                std::string mimeType = getMimeType(ext);

                auto fileSize = std::filesystem::file_size(filePath);

                std::ifstream file(filePath, std::ios::binary);

                std::string headers =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: " + mimeType + "\r\n"
                    "Content-Length: " +
                    std::to_string(fileSize) + "\r\n"
                                               "Connection: close\r\n"
                                               "\r\n";
                send(clientSocket, headers.c_str(), (int)headers.size(), 0);

                char fileBuffer[BUFFER_SIZE];
                while (file)
                {
                    file.read(buffer, sizeof(fileBuffer));
                    std::streamsize bytesRead = file.gcount();

                    if (bytesRead > 0)
                    {
                        send(clientSocket, buffer, (int)bytesRead, 0);
                    }
                }
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