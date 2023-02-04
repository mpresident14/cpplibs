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


enum class HttpCode {
  OK = 200,
  BAD_REQUEST = 400,
  INTERNAL = 500,
};

const std::unordered_map<HttpCode, const char*> HTTP_CODES =
    std::unordered_map<HttpCode, const char*>{
        {HttpCode::OK, "OK"},
        {HttpCode::BAD_REQUEST, "BAD_REQUEST"},
        {HttpCode::INTERNAL, "INTERNAL"},
    };

class HttpRequest {

public:
  static HttpRequest parse(const std::string& str);

  const std::string method_;
  const std::string url_;
  const std::string version_;
  const std::unordered_map<std::string, std::string> headers_;


private:
  HttpRequest(
      std::string_view method,
      std::string_view url,
      std::string_view version,
      std::unordered_map<std::string, std::string>&& headers);


  static void check_first_line_end(const auto& iter, const auto& end) {
    if (iter == end) {
      throw std::invalid_argument(
          "Invalid HTTP Request: Expected 3 space-separated chunks on first line.");
    }
  }
};

class HttpResponse {

public:
  HttpResponse(HttpCode code, std::string_view body);
  friend std::ostream& operator<<(std::ostream& out, const HttpResponse& c);

private:
  HttpCode code_;
  std::string body_;
};

std::ostream& operator<<(std::ostream& out, const HttpResponse& response);


} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_REQUEST_HPP
