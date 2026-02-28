#pragma once
#include <string>

// Decodes %XX and '+' => space
std::string urlDecode(const std::string& s);

// Returns false if path attempts traversal; output becomes a safe normalized web path.
// Expects a web path like "/img/a.png". Keeps it absolute-ish (leading slash).
bool sanitizeWebPath(const std::string& decodedPath, std::string& safePath);