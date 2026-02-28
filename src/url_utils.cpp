#include "url_utils.h"
#include <cctype>
#include <sstream>
#include <vector>

std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            auto hex1 = str[i + 1];
            auto hex2 = str[i + 2];
            auto isHex = [](char c){ return std::isxdigit((unsigned char)c) != 0; };
            if (isHex(hex1) && isHex(hex2)) {
                std::string hex = str.substr(i + 1, 2);
                char decoded_char = (char)std::stoi(hex, nullptr, 16);
                result += decoded_char;
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

// Basic, safe path normalization for web paths.
// - must start with '/'
// - reject any segment ".."
// - collapse "." and empty segments
bool sanitizeWebPath(const std::string& decodedPath, std::string& safePath) {
    if (decodedPath.empty()) return false;
    if (decodedPath[0] != '/') return false;

    // Split by '/'
    std::vector<std::string> segments;
    std::string cur;
    for (char c : decodedPath) {
        if (c == '/') {
            if (!cur.empty()) segments.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) segments.push_back(cur);

    std::vector<std::string> out;
    for (const auto& seg : segments) {
        if (seg == "." || seg.empty()) continue;
        if (seg == "..") return false;
        // optional: block backslashes
        if (seg.find('\\') != std::string::npos) return false;
        out.push_back(seg);
    }

    std::ostringstream oss;
    oss << "/";
    for (size_t i = 0; i < out.size(); ++i) {
        oss << out[i];
        if (i + 1 < out.size()) oss << "/";
    }
    safePath = oss.str();
    return true;
}