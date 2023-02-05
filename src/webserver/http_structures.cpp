#include "src/webserver/http_structures.hpp"

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

namespace {

const std::regex NO_SPACE_REGEX("[^\\s]+");
const std::regex HEADER_REGEX("([^\\s]+)\\s*:\\s*([^\r\n]+)");
// Path is (/(?:[^?]+))
// (optional) query params are (?:\\?([0-9a-zA-Z_-]+=[0-9a-zA-Z_-]+))?
// Note that browser does not send anchor to server:
// https://stackoverflow.com/questions/3067491/is-the-anchor-part-of-a-url-being-sent-to-a-web-server#:~:text=%22Note%20that%20the%20fragment%20identifier,interpreted%20locally%20within%20the%20browser%22.
const std::regex URL_REGEX("(\\/(?:[^?]+)?)(?:\\?([0-9a-zA-Z_-]+=[0-9a-zA-Z_-]+))?");

} // namespace


HttpRequest::HttpRequest(
    std::string_view method,
    std::string path,
    std::string query_params,
    std::string_view version,
    std::unordered_map<std::string, std::string>&& headers)
    : method_(method), path_(path), query_params_(query_params), version_(version),
      headers_(std::move(headers)) {}


HttpRequest HttpRequest::parse(const std::string& str) {
  size_t end_of_first_line = str.find('\n');
  if (end_of_first_line == std::string::npos) {
    end_of_first_line = str.find('\r');
  }
  if (end_of_first_line == std::string::npos) {
    throw std::invalid_argument("Invalid HTTP Request: no line breaks");
  }

  auto first_line_iter =
      std::sregex_iterator(str.cbegin(), str.cbegin() + end_of_first_line, NO_SPACE_REGEX);
  auto end = std::sregex_iterator();
  check_first_line_end(first_line_iter, end);
  std::string method = (*first_line_iter++)[0];
  check_first_line_end(first_line_iter, end);
  std::string url = (*first_line_iter++)[0];
  check_first_line_end(first_line_iter, end);
  std::string version = (*first_line_iter++)[0];

  std::smatch url_match;
  if (!std::regex_match(url, url_match, URL_REGEX)) {
    throw std::invalid_argument("Invalid HTTP Request: malformed URL");
  }

  const auto& path = url_match[1];
  const auto& query_params = url_match[2];


  std::unordered_map<std::string, std::string> headers;
  for (auto iter = std::sregex_iterator(str.cbegin() + end_of_first_line, str.cend(), HEADER_REGEX);
       iter != end;
       ++iter) {
    const std::smatch& match = *iter;
    headers.emplace(match[1], match[2]);
  }

  return HttpRequest(method, path, query_params, version, std::move(headers));
}


HttpResponse::HttpResponse(HttpCode code, std::string_view body) : code_(code), body_(body) {}


std::ostream& operator<<(std::ostream& out, const HttpResponse& response) {
  out << "HTTP/1.1 " << static_cast<int>(response.code_) << ' ' << HTTP_CODES.at(response.code_)
      << "\n\n"
      << response.body_;
  return out;
}


} // namespace prez::webserver
