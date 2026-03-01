#include "http_request.h"
#include <url_utils.h>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

static inline std::string trim(std::string s)
{
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static inline std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

static bool getHeader(const std::map<std::string, std::string>& h,
                      const std::string& keyLower,
                      std::string& outVal)
{
    auto it = h.find(keyLower);
    if (it == h.end()) return false;
    outVal = it->second;
    return true;
}

static bool parseContentLength(const std::map<std::string, std::string>& h, std::size_t& outLen)
{
    std::string v;
    if (!getHeader(h, "content-length", v)) return false;
    v = trim(v);
    if (v.empty()) return false;

    // no exceptions: use strtoull
    char* end = nullptr;
    unsigned long long n = std::strtoull(v.c_str(), &end, 10);
    if (end == v.c_str() || *end != '\0') return false;
    outLen = static_cast<std::size_t>(n);
    return true;
}

static bool isChunked(const std::map<std::string, std::string>& h)
{
    std::string v;
    if (!getHeader(h, "transfer-encoding", v)) return false;
    v = toLower(v);
    return v.find("chunked") != std::string::npos;
}

// Minimal chunked decoder: expects proper CRLF framing.
// Returns false on malformed data.
static bool decodeChunked(const std::string& in, std::string& out)
{
    out.clear();
    std::size_t i = 0;

    while (true)
    {
        // read chunk size line: hex[;extensions]\r\n
        std::size_t lineEnd = in.find("\r\n", i);
        if (lineEnd == std::string::npos) return false;

        std::string sizeLine = in.substr(i, lineEnd - i);
        auto semi = sizeLine.find(';');
        if (semi != std::string::npos) sizeLine = sizeLine.substr(0, semi);
        sizeLine = trim(sizeLine);
        if (sizeLine.empty()) return false;

        char* end = nullptr;
        unsigned long long chunkSize = std::strtoull(sizeLine.c_str(), &end, 16);
        if (end == sizeLine.c_str() || *end != '\0') return false;

        i = lineEnd + 2; // after \r\n

        if (chunkSize == 0)
        {
            // final chunk: may have trailer headers ending with \r\n
            // We can ignore trailers; just ensure we have at least \r\n after trailers or allow end.
            // If there are trailers, they end with \r\n\r\n.
            // It's okay to not strictly validate trailers here.
            return true;
        }

        if (i + chunkSize > in.size()) return false;
        out.append(in.data() + i, static_cast<std::size_t>(chunkSize));
        i += static_cast<std::size_t>(chunkSize);

        // chunk data must be followed by \r\n
        if (i + 1 >= in.size()) return false;
        if (in.compare(i, 2, "\r\n") != 0) return false;
        i += 2;
    }
}

static std::string headerValueBeforeSemicolon(const std::string& v)
{
    auto semi = v.find(';');
    return trim(semi == std::string::npos ? v : v.substr(0, semi));
}

static void parseUrlEncodedForm(const std::string& body, std::map<std::string,std::string>& outForm)
{
    outForm.clear();
    std::size_t start = 0;
    while (start <= body.size())
    {
        std::size_t amp = body.find('&', start);
        std::string pair = (amp == std::string::npos) ? body.substr(start) : body.substr(start, amp - start);

        if (!pair.empty())
        {
            std::size_t eq = pair.find('=');
            std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
            std::string v = (eq == std::string::npos) ? ""   : pair.substr(eq + 1);

            // application/x-www-form-urlencoded uses '+' for spaces
            std::replace(k.begin(), k.end(), '+', ' ');
            std::replace(v.begin(), v.end(), '+', ' ');

            k = urlDecode(k);
            v = urlDecode(v);

            outForm[k] = v;
        }

        if (amp == std::string::npos) break;
        start = amp + 1;
    }
}

bool parseHttpRequest(const std::string &raw, HttpRequest &out)
{
    // Split headers/body
    auto pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos)
        return false;

    std::string head = raw.substr(0, pos);
    std::string bodyPart = raw.substr(pos + 4); // everything after header terminator
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

    // NEW: parse Content-Type / Content-Length
    out.contentLength = 0;
    out.contentType.clear();
    out.body.clear();
    out.form.clear();
    out.json.clear();

    std::string ct;
    if (getHeader(out.headers, "content-type", ct))
        out.contentType = trim(ct);

    std::size_t cl = 0;
    bool hasCL = parseContentLength(out.headers, cl);
    if (hasCL) out.contentLength = cl;

    // NEW: decode/extract body
    if (isChunked(out.headers))
    {
        std::string decoded;
        if (!decodeChunked(bodyPart, decoded))
            return false;
        out.body = std::move(decoded);
        out.contentLength = out.body.size(); // effective length
    }
    else if (hasCL)
    {
        // If raw may contain extra bytes (pipelining), only take CL bytes if available
        if (bodyPart.size() < cl)
            return false; // incomplete request
        out.body.assign(bodyPart.data(), cl);
    }
    else
    {
        // No CL and not chunked:
        // - For many requests (GET) body is empty
        // - For others, we can take whatever is present in raw
        out.body = bodyPart;
        out.contentLength = out.body.size();
    }

    // OPTIONAL: parse common body content
    std::string mediaType = toLower(headerValueBeforeSemicolon(out.contentType));
    if (mediaType == "application/x-www-form-urlencoded")
    {
        parseUrlEncodedForm(out.body, out.form);
    }
    else if (mediaType == "application/json")
    {
        // keep raw JSON string (parsing requires a JSON lib)
        out.json = out.body;
    }

    return true;
}