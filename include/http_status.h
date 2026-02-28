#pragma once

enum class HttpStatus : int
{
  OK = 200,
  Found = 302,
  BadRequest = 400,
  NotFound = 404,
  MethodNotAllowed = 405,
  InternalServerError = 500,
  NotImplemented = 501,
};

inline const char *reasonPhrase(HttpStatus s)
{
  switch (s)
  {
  case HttpStatus::OK:
    return "OK";
  case HttpStatus::Found:
    return "Found";
  case HttpStatus::BadRequest:
    return "Bad Request";
  case HttpStatus::NotFound:
    return "Not Found";
  case HttpStatus::MethodNotAllowed:
    return "Method Not Allowed";
  case HttpStatus::InternalServerError:
    return "Internal Server Error";
  case HttpStatus::NotImplemented:
    return "Not Implemented";
  default:
    return "Unknown";
  }
}