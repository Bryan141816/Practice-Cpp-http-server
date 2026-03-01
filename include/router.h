#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>

struct HttpRequest;
struct HttpResponse;

// TODO: enable dynamic routes

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

enum class MatchError
{
  None,
  NotFound,
  MethodNotAllowed
};

struct MatchResult
{
  const Route *route = nullptr;
  MatchError error = MatchError::NotFound;
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
  void post(const std::string &path, Handler h)
  {
    addRoute(HttpMethod::POST, path, std::move(h));
  }

  static bool pathMatches(const std::string &pattern, const std::string &path)
  {
    if (pattern == path)
      return true;

    if (!path.empty() && path.back() != '/')
    {
      if (pattern == path + "/")
        return true;
    }
    if (path.size() > 1 && path.back() == '/')
    {
      std::string without = path;
      without.pop_back();
      if (pattern == without)
        return true;
    }
    return false;
  }

  MatchResult match(HttpMethod method, const std::string &path) const
  {
    bool pathFound = false;

    for (const auto &route : routes_)
    {
      if (!pathMatches(route.pathPattern, path))
        continue;

      pathFound = true;

      if (route.method == method)
        return MatchResult{&route, MatchError::None};
    }

    if (pathFound)
      return MatchResult{nullptr, MatchError::MethodNotAllowed};

    return MatchResult{nullptr, MatchError::NotFound};
  }
  const std::vector<Route> &getAllRoute() const { return routes_; }

private:
  std::string prefix_;

protected:
  std::vector<Route> routes_;
};