#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>
struct HttpRequest;
struct HttpResponse;

enum class HttpMethod
{
  GET,
  POST,
  PUT,
  DELETE_,
  UNKNOWN,
};

struct Route
{
  HttpMethod method;
  std::string pathPattern;
  std::function<HttpResponse(const HttpRequest &)> handler;
};

class Router
{
public:
  using Handler = std::function<HttpResponse(const HttpRequest &)>;

  Router(std::string prefix = "") : prefix_(prefix) {}

  void addRoute(HttpMethod method, const std::string &path, Handler handler)
  {
    routes_.push_back({method,
                       prefix_ + path,
                       std::move(handler)});
  }

  void get(const std::string &path, Handler h)
  {
    addRoute(HttpMethod::GET, path, std::move(h));
  }

  // returns nullptr if not found
  const Route *match(HttpMethod method,
                     const std::string &path) const
  {

    for (const auto &route : routes_)
    {

      if (route.method == method)
      {
        if (route.pathPattern == path)
        {
          return &route;
        }
        if (!path.empty() && path.back() != '/')
        {
          std::string withSlash = path + "/";
          if (route.pathPattern == withSlash)
            return &route;
        }
        if (path.size() > 1 && path.back() == '/')
        {
          std::string withoutSlash = path;
          withoutSlash.pop_back();

          std::cout << (route.pathPattern == withoutSlash) << std::endl;
          if (route.pathPattern == withoutSlash)
            return &route;
        }
      }
    }
    return nullptr;
  }
  const std::vector<Route> &getAllRoute() const { return routes_; }

private:
  std::string prefix_;

protected:
  std::vector<Route> routes_;
};