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
    Router router{};

    router.post("/test", [](const HttpRequest &req)
                {   
                    std::cout << req.body << std::endl;
                    return redirectTo("/index.html"); });

    ServerHandler server(PORT, WORKING_DIR);
    server.registerRoute(router);
    if (server.start())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}