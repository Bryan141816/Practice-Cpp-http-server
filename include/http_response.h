#pragma once
#include "http_status.h"
#include <string>
#include <map>
#include <winsock2.h>

struct HttpResponse {
    HttpStatus status {HttpStatus::OK};
    std::map<std::string, std::string> headers;
    std::string body;

    std::string toString() const;
};

bool sendAll(SOCKET s, const char* data, int len);
bool sendResponse(SOCKET clientSocket, const HttpResponse& resp);

// Common helpers
HttpResponse makeText(HttpStatus st, const std::string& text, const std::string& mime = "text/plain; charset=utf-8");
HttpResponse redirectTo(const std::string& location);
HttpResponse respondNotFound();
HttpResponse respondMethodNotAllowed();
HttpResponse respondServerError();