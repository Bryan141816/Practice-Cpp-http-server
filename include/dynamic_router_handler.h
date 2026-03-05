#pragma once
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <optional>
#include <cctype>

namespace dynamic_route
{

  enum class ParamType
  {
    Int,
    Float,
    Str,
    Bool
  };

  using ParamValue = std::variant<int, double, std::string, bool>;
  using ParamsMap = std::unordered_map<std::string, ParamValue>;

  struct CompiledRoute
  {
    std::regex re;
    std::vector<std::string> names; // capture group index -> param name (1-based in match_results)
    std::vector<ParamType> types;   // capture group index -> param type
  };

  static std::string toLower(std::string s);

  static ParamType parseType(const std::string &t);

  static std::string patternForType(ParamType t);
  // Escape regex metacharacters in literal segments
  static std::string regexEscape(const std::string &s);

  CompiledRoute compileRoute(const std::string &registeredPath);

  std::optional<ParamsMap> matchAndParse(const std::string &registeredPath,
                                         const std::string &receivedPath);
  bool isDynamicRoutePattern(const std::string &s);
}