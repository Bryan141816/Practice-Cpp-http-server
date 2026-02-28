#include "server_handler.h"

ServerHandler::ServerHandler(int port, std::string dir)
    : port(port), workingDir(std::move(dir)) {}
void ServerHandler::addRouter(Router router)
{
  this->router = router;
}
bool ServerHandler::start()
{
  WSADATA wsa{};
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
  {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }

  SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET)
  {
    std::cerr << "Failed to create socket\n";
    WSACleanup();
    return 1;
  }

  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
  {
    std::cerr << "Failed to bind socket\n";
    closesocket(serverSocket);
    WSACleanup();
    return false;
  }

  if (listen(serverSocket, 16) == SOCKET_ERROR)
  {
    std::cerr << "Failed to listen\n";
    closesocket(serverSocket);
    WSACleanup();
    return false;
  }

  std::cout << "Server listening on http://localhost:" << port << "\n";

  while (true)
  {
    sockaddr_in clientAddr{};
    int clientLen = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
    if (clientSocket == INVALID_SOCKET)
    {
      std::cerr << "Failed to accept\n";
      continue;
    }

    std::string rawRequest;
    if (!recvUntilHeaders(clientSocket, rawRequest))
    {
      closesocket(clientSocket);
      continue;
    }

    HttpRequest request;
    if (!parseHttpRequest(rawRequest, request))
    {
      sendResponse(clientSocket, makeText(HttpStatus::BadRequest, "Bad Request\n"));
      closesocket(clientSocket);
      continue;
    }

    // Only allow GET
    if (request.method != "GET")
    {
      sendResponse(clientSocket, respondMethodNotAllowed());
      closesocket(clientSocket);
      continue;
    }

    // Handle routing
    std::string path = request.path;

    if (const Router::Handler *h = router.matchGet(request.path))
    {
      HttpResponse resp = (*h)(request);
      sendResponse(clientSocket, resp);
      closesocket(clientSocket);
      continue;
    }

    // Sanitize path
    std::string safePath;
    if (!sanitizeWebPath(path, safePath))
    {
      sendResponse(clientSocket, makeText(HttpStatus::BadRequest, "Bad Request\n"));
      closesocket(clientSocket);
      continue;
    }

    std::filesystem::path filePath = std::filesystem::path(workingDir) / safePath.substr(1);

    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath))
    {
      sendResponse(clientSocket, respondNotFound());
      closesocket(clientSocket);
      continue;
    }

    std::string body;
    if (!readfiletoString(filePath, body))
    {
      sendResponse(clientSocket, respondServerError());
      closesocket(clientSocket);
      continue;
    }

    HttpResponse ok;
    ok.status = HttpStatus::OK;
    ok.headers["Content-Type"] = getMimeType(filePath.extension().string());
    ok.body = std::move(body);

    sendResponse(clientSocket, ok);
    closesocket(clientSocket);
  }

  closesocket(serverSocket);
  WSACleanup();
  return true;
}

bool ServerHandler::recvUntilHeaders(SOCKET s, std::string &out)
{
  out.clear();
  char buf[bufferSize];

  while (true)
  {
    int n = recv(s, buf, bufferSize, 0);
    if (n <= 0)
      return false;
    out.append(buf, n);
    if (out.find("\r\n\r\n") != std::string::npos)
      return true;
  }
}