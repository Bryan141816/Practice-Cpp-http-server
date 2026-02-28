#include <http_response.h>
#include <sstream>

std::string HttpResponse::toString() const {
    std::ostringstream out;

    out << "HTTP/1.1 " << (int)status << " " << httpstatus::reasonPhrase(status) << "\r\n";

    auto hasHeader = [&](const std::string& key) {
        return headers.find(key) != headers.end();
    };

    // Defaults
    if (!hasHeader("Connection")) {
        out << "Connection: close\r\n";
    }

    if (!hasHeader("Content-Length")) {
        out << "Content-Length: " << body.size() << "\r\n";
    }

    // Emit other headers
    for (const auto& kv : headers) {
        const std::string& k = kv.first;
        const std::string& v = kv.second;

        if (k == "Connection" || k == "Content-Length")
            continue;

        out << k << ": " << v << "\r\n";
    }

    // Emit explicitly set ones
    if (hasHeader("Content-Length")) {
        out << "Content-Length: " << headers.at("Content-Length") << "\r\n";
    }

    if (hasHeader("Connection")) {
        out << "Connection: " << headers.at("Connection") << "\r\n";
    }

    out << "\r\n";
    out << body;

    return out.str();
}

bool sendAll(SOCKET s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        int n = send(s, data + sent, len - sent, 0);
        if (n == SOCKET_ERROR || n == 0) return false;
        sent += n;
    }
    return true;
}

bool sendResponse(SOCKET clientSocket, const HttpResponse& resp) {
    std::string raw = resp.toString();
    return sendAll(clientSocket, raw.c_str(), (int)raw.size());
}

HttpResponse makeText(httpstatus::HttpStatus st, const std::string& text, const std::string& mime) {
    HttpResponse r;
    r.status = st;
    r.headers["Content-Type"] = mime;
    r.body = text;
    return r;
}

HttpResponse redirectTo(const std::string& location) {
    HttpResponse r;
    r.status = httpstatus::HttpStatus::Found;
    r.headers["Location"] = location;
    r.body = "";
    return r;
}

HttpResponse respondNotFound() {
    return makeText(httpstatus::HttpStatus::NotFound,
        "<html><body><h1>404 Not Found</h1></body></html>",
        "text/html; charset=utf-8"
    );
}

HttpResponse respondMethodNotAllowed() {
    HttpResponse r = makeText(httpstatus::HttpStatus::MethodNotAllowed,
        "<html><body><h1>405 Method Not Allowed</h1></body></html>",
        "text/html; charset=utf-8"
    );
    r.headers["Allow"] = "GET";
    return r;
}

HttpResponse respondServerError() {
    return makeText(httpstatus::HttpStatus::InternalServerError,
        "<html><body><h1>500 Internal Server Error</h1></body></html>",
        "text/html; charset=utf-8"
    );
}