#include <dynamic_router_handler.h>
namespace dynamic_route{
  static std::string toLower(std::string s)
  {
    for (char &c : s)
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
  }

  static ParamType parseType(const std::string &t)
  {
    if (t == "int")
      return ParamType::Int;
    if (t == "float")
      return ParamType::Float;
    if (t == "str")
      return ParamType::Str;
    if (t == "bool")
      return ParamType::Bool;
    throw std::runtime_error("Unknown type: " + t);
  }

  static std::string patternForType(ParamType t)
  {
    switch (t)
    {
    case ParamType::Int:
      return R"(-?\d+)";
    case ParamType::Float:
      return R"(-?(?:\d+(?:\.\d*)?|\.\d+))";
    case ParamType::Str:
      return R"([^/]+)";
    case ParamType::Bool:
      return R"((?:true|false|1|0))";
    }
    return "";
  }

  // Escape regex metacharacters in literal segments
  static std::string regexEscape(const std::string &s)
  {
    static const std::string metachars = R"(\.^$|()[]{}*+?!)";
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s)
    {
      if (metachars.find(c) != std::string::npos)
        out.push_back('\\');
      out.push_back(c);
    }
    return out;
  }

  CompiledRoute compileRoute(const std::string &registeredPath)
  {
    // Matches: <int:id>, <float:x>, <str:name>, <bool:ok>
    static const std::regex paramRe(R"(<(int|float|str|bool):([A-Za-z_][A-Za-z0-9_]*)>)");

    CompiledRoute cr{std::regex(""), {}, {}};

    std::string regexSrc;
    regexSrc.reserve(registeredPath.size() * 2);

    std::sregex_iterator it(registeredPath.begin(), registeredPath.end(), paramRe);
    std::sregex_iterator end;

    size_t lastPos = 0;

    for (; it != end; ++it)
    {
      const std::smatch &m = *it;

      // literal chunk before the param
      size_t pos = static_cast<size_t>(m.position());

      regexSrc += regexEscape(registeredPath.substr(lastPos, pos - lastPos));

      std::string typeStr = m.str(1);
      std::string nameStr = m.str(2);

      ParamType pt = parseType(typeStr);
      cr.names.push_back(nameStr);
      cr.types.push_back(pt);

      // Add a capturing group (C++ doesn't do named groups)
      regexSrc += "(" + patternForType(pt) + ")";

      lastPos = pos + static_cast<size_t>(m.length());
    }

    // trailing literal chunk
    regexSrc += regexEscape(registeredPath.substr(lastPos));

    // full match
    regexSrc = "^" + regexSrc + "$";

    cr.re = std::regex(regexSrc, std::regex::ECMAScript);
    return cr;
  }

  std::optional<ParamsMap> matchAndParse(const std::string &registeredPath,
                                         const std::string &receivedPath)
  {
    CompiledRoute cr = compileRoute(registeredPath);

    std::smatch match;
    if (!std::regex_match(receivedPath, match, cr.re))
    {
      return std::nullopt;
    }

    ParamsMap params;

    std::cout << "Match: " << match[0].str() << std::endl;
    for (size_t i = 0; i < cr.names.size(); ++i)
    {
      std::string raw = match[i + 1].str();
      const auto &name = cr.names[i];
      ParamType t = cr.types[i];

      switch (t)
      {
      case ParamType::Int:
        params[name] = std::stoi(raw);
        break;
      case ParamType::Float:
        params[name] = std::stod(raw);
        break;
      case ParamType::Str:
        params[name] = raw;
        break;
      case ParamType::Bool:
      {
        std::string v = toLower(raw);
        params[name] = (v == "true" || v == "1");
        break;
      }
      }
    }

    return params;
  }
  bool isDynamicRoutePattern(const std::string &s)
  {
    static const std::regex paramRe(R"(<(int|float|str|bool):([A-Za-z_][A-Za-z0-9_]*)>)");
    return std::regex_search(s, paramRe);
  }
}