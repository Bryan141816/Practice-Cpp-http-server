#pragma once
#include <string>
#include <unordered_map>
#include <functional>

struct HttpRequest;
struct HttpResponse;

class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    class Group {
    public:
        Group(Router& r, std::string prefix) : r_(r), prefix_(std::move(prefix)) {}

        void get(const std::string& path, Handler h) {
            r_.get(join(prefix_, path), std::move(h));
        }

    private:
        static std::string join(const std::string& a, const std::string& b) {
            if (a.empty()) return b;
            if (b.empty()) return a;
            if (a.back() == '/' && b.front() == '/') return a + b.substr(1);
            if (a.back() != '/' && b.front() != '/') return a + "/" + b;
            return a + b;
        }

        Router& r_;
        std::string prefix_;
    };

    Group group(const std::string& prefix) { return Group(*this, prefix); }

    void get(const std::string& path, Handler h) {
        getRoutes_[path] = std::move(h);
    }

    // returns nullptr if not found
    const Handler* matchGet(const std::string& path) const {
        auto it = getRoutes_.find(path);
        if (it == getRoutes_.end()) return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<std::string, Handler> getRoutes_;
};