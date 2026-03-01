#pragma once
#include <string>
#include <unordered_map>
#include <functional>

struct HttpRequest;
struct HttpResponse;

class Router
{
public:
  using Handler = std::function<HttpResponse(const HttpRequest &)>;

  Router(std::string prefix = "") : prefix_(prefix) {}

  void get(const std::string &path, Handler h)
  {
    getRoutes_[prefix_ + path] = std::move(h);
  }

  // returns nullptr if not found
  const Handler *matchGet(const std::string &path) const
  {
    auto it = getRoutes_.find(path);
    if (it == getRoutes_.end())
      return nullptr;
    return &it->second;
  }

private:
  std::string prefix_;
  std::unordered_map<std::string, Handler> getRoutes_;
};