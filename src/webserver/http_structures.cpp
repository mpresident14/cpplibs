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

void check_first_line_end(const auto& iter, const auto& end) {
  if (iter == end) {
    throw std::invalid_argument(
        "Invalid HTTP Request: Expected 3 space-separated chunks on first line.");
  }
}

} // namespace

const std::unordered_map<HttpRequest::Method, const char*> HttpRequest::METHODS = {
    {Method::GET, "GET"}};
const std::unordered_map<std::string, HttpRequest::Method> HttpRequest::METHOD_NAMES = {
    {"GET", Method::GET}};

HttpRequest::HttpRequest(
    Method method,
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

  auto method_entry = METHOD_NAMES.find(method);
  if (method_entry == METHOD_NAMES.end()) {
    throw std::invalid_argument("Invalid HTTP Request: Unknown HTTP Method: " + method);
  }

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

  return HttpRequest(method_entry->second, path, query_params, version, std::move(headers));
}

std::ostream& operator<<(std::ostream& out, const HttpRequest& request) {
  out << HttpRequest::METHODS.at(request.method_) << ' ' << request.path_ << ' '
      << request.version_;
  for (const auto& [key, val] : request.headers_) {
    out << '\n' << key << " : " << val;
  }
  return out;
}

const std::unordered_map<HttpResponse::Code, const char*> HttpResponse::CODES = {
    {Code::OK, "OK"},
    {Code::BAD_REQUEST, "BAD_REQUEST"},
    {Code::NOT_FOUND, "NOT_FOUND"},
    {Code::INTERNAL, "INTERNAL"},
};

HttpResponse::HttpResponse(HttpResponse::Code code, std::string_view body)
    : code_(code), body_(body) {}


std::ostream& operator<<(std::ostream& out, const HttpResponse& response) {
  out << "HTTP/1.1 " << static_cast<int>(response.code_) << ' '
      << HttpResponse::CODES.at(response.code_) << "\n\n"
      << response.body_;
  return out;
}


} // namespace prez::webserver
