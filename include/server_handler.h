#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "http_response.h"
#include "http_request.h"
#include "mimetype_utils.h"
#include "url_utils.h"
#include "file_handler.h"
#include "global_router.h"

class GlobalRouter;

class ServerHandler {
public:
  ServerHandler(int port, std::string dir);
  void registerRoute(Router router);
  bool start();

private:
  int port;
  std::string workingDir;
  GlobalRouter router;

  static constexpr int bufferSize = 4096;
  static bool recvUntilHeaders(SOCKET s, std::string &out);
  static void logger(std::string status, std::string &path);
  static HttpMethod parseMethod(std::string m);
};