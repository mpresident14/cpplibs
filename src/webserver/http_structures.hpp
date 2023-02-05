#ifndef PREZ_WEBSERVER_HTTP_REQUEST_HPP
#define PREZ_WEBSERVER_HTTP_REQUEST_HPP

#define stringify(name) #name

#include <cctype>
#include <functional>
#include <iostream>
#include <locale>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace prez::webserver {


class HttpRequest {

public:
  enum class Method { GET, POST };

  static const std::unordered_map<Method, const char*> METHODS;
  static const std::unordered_map<const char*, Method> METHOD_NAMES;

  static HttpRequest parse(const std::string& str);

  const Method method_;
  const std::string path_;
  const std::string query_params_;
  const std::string version_;
  const std::unordered_map<std::string, std::string> headers_;

  friend std::ostream& operator<<(std::ostream& out, const HttpRequest& request);


private:
  HttpRequest(
      Method method,
      std::string path,
      std::string query_params,
      std::string_view version,
      std::unordered_map<std::string, std::string>&& headers);
};

std::ostream& operator<<(std::ostream& out, const HttpRequest& request);


class HttpResponse {

public:
  enum class Code {
    OK = 200,
    BAD_REQUEST = 400,
    INTERNAL = 500,
  };

  static const std::unordered_map<Code, const char*> CODES;

  HttpResponse(Code code, std::string_view body);
  friend std::ostream& operator<<(std::ostream& out, const HttpResponse& c);


private:
  Code code_;
  std::string body_;
};

std::ostream& operator<<(std::ostream& out, const HttpResponse& response);


} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_REQUEST_HPP
