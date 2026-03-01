#include "server_handler.h"

ServerHandler::ServerHandler(int port, std::string dir)
    : port(port), workingDir(std::move(dir)) {}
void ServerHandler::registerRoute(Router router)
{
  this->router.includeRouter(router);
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
      sendResponse(clientSocket, makeText(httpstatus::HttpStatus::BadRequest, "Bad Request\n"));
      closesocket(clientSocket);
      logger("400", request.path);
      continue;
    }


    std::string path = request.path;
    HttpMethod method = parseMethod(request.method);
    MatchResult result = router.match(method, request.path);
    if (result.route)
    {
      const Route *route = result.route;
      HttpResponse resp = route->handler(request);
      sendResponse(clientSocket, resp);
      closesocket(clientSocket);

      std::string stringStatus = httpstatus::toString(resp.status);
      logger(stringStatus, request.path);
      continue;
    }
    else if (result.error == MatchError::MethodNotAllowed)
    {
      {
        sendResponse(clientSocket, respondMethodNotAllowed());
        closesocket(clientSocket);
        logger("405", request.path);
        continue;
      }
    }

    // Sanitize path
    std::string safePath;
    if (!sanitizeWebPath(path, safePath))
    {
      sendResponse(clientSocket, makeText(httpstatus::HttpStatus::BadRequest, "Bad Request\n"));
      closesocket(clientSocket);
      logger("400", request.path);
      continue;
    }

    std::filesystem::path filePath = std::filesystem::path(workingDir) / safePath.substr(1);

    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath))
    {
      sendResponse(clientSocket, respondNotFound());
      closesocket(clientSocket);
      logger("404", request.path);
      continue;
    }

    std::string body;
    if (!readfiletoString(filePath, body))
    {
      sendResponse(clientSocket, respondServerError());
      closesocket(clientSocket);
      logger("500", request.path);
      continue;
    }

    HttpResponse ok;
    ok.status = httpstatus::HttpStatus::OK;
    ok.headers["Content-Type"] = getMimeType(filePath.extension().string());
    ok.body = std::move(body);

    sendResponse(clientSocket, ok);
    logger("200", request.path);
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
void ServerHandler::logger(std::string status, std::string &path)
{
  std::cout << status << ": " << path << std::endl;
}
HttpMethod ServerHandler::parseMethod(std::string m)
{
  if (m == "GET")
    return HttpMethod::GET;
  if (m == "POST")
    return HttpMethod::POST;
  if (m == "PUT")
    return HttpMethod::PUT;
  if (m == "DELETE")
    return HttpMethod::DELETE_;
  return HttpMethod::UNKNOWN;
}