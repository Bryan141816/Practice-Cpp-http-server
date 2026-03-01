#pragma once
#include <string>
#include <map>
#include <cstddef>

struct HttpRequest
{
  std::string method;
  std::string target;
  std::string path;
  std::string query;
  std::string version;
  std::map<std::string, std::string> headers;

  // NEW
  std::string body;               // raw body bytes (as received/decoded)
  std::size_t contentLength = 0;  // parsed Content-Length if present
  std::string contentType;        // parsed Content-Type (may include charset)

  // OPTIONAL: parsed content helpers
  std::map<std::string, std::string> form; // application/x-www-form-urlencoded
  std::string json;                         // application/json (raw, not parsed to object)
};

bool parseHttpRequest(const std::string &raw, HttpRequest &out);