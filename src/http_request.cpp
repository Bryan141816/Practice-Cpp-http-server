#include "http_request.h"
#include <url_utils.h>
#include <sstream>
#include <algorithm>

static inline std::string trim(std::string s)
{
    auto notSpace = [](unsigned char c)
    { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static inline std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                   { return (char)std::tolower(c); });
    return s;
}

bool parseHttpRequest(const std::string &raw, HttpRequest &out)
{
    // Split headers section only
    auto pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos)
        return false;

    std::string head = raw.substr(0, pos);
    std::istringstream ss(head);

    // Request line
    std::string requestLine;
    if (!std::getline(ss, requestLine))
        return false;
    if (!requestLine.empty() && requestLine.back() == '\r')
        requestLine.pop_back();

    std::istringstream rl(requestLine);
    if (!(rl >> out.method >> out.target >> out.version))
        return false;

    // Parse headers
    out.headers.clear();
    std::string line;
    while (std::getline(ss, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break;
        auto colon = line.find(':');
        if (colon == std::string::npos)
            continue;
        std::string key = toLower(trim(line.substr(0, colon)));
        std::string val = trim(line.substr(colon + 1));
        out.headers[key] = val;
    }

    // Split path/query
    std::string path = out.target;
    std::string query;
    auto qpos = out.target.find('?');
    if (qpos != std::string::npos)
    {
        path = out.target.substr(0, qpos);
        query = out.target.substr(qpos + 1);
    }

    out.path = urlDecode(path);
    out.query = query;
    return true;
}