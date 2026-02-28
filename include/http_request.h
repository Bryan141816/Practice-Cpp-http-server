#pragma once
#include <string>
#include <map>

struct HttpRequest
{
  std::string method;
  std::string target;
  std::string path;
  std::string query;
  std::string version;
  std::map<std::string, std::string> headers;
};
bool parseHttpRequest(const std::string &raw, HttpRequest &out);
