#include <iostream>
#include "http_response.h"
#include "http_request.h"
#include "router.h"
#include "server_handler.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096
#define WORKING_DIR "E:\\CODES\\Practice-http\\public"

int main()
{
    Router router;
    router.get("/", [](const HttpRequest &)
               { return redirectTo("/index.html"); });
    router.get("/cheval", [](const HttpRequest &)
               { return redirectTo("/second.html"); });

    ServerHandler server(PORT, WORKING_DIR);
    server.addRouter(router);
    server.start();
    return 0;
}