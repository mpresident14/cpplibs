#ifndef PREZ_WEBSERVER_HTTP_HANDLER_HPP
#define PREZ_WEBSERVER_HTTP_HANDLER_HPP

#include "src/webserver/http_structures.hpp"

#include <concepts>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

namespace prez::webserver {


class HttpHandler {
public:
  template <typename F>
    requires std::convertible_to<std::result_of_t<F(const HttpRequest&)>, HttpResponse>
  static std::unique_ptr<HttpHandler> fromFunction(F&& lambda);

  static HttpResponse loadFile(const std::string& url_path) {
    // TODO: Make this secure: no absolute paths, dots, etc.
    // TODO: assert len(url_path) > 1
    // Skip the initial / in the URL path.
    std::ifstream fstrm(url_path.c_str() + 1);
    if (fstrm.is_open()) {
      return HttpResponse(
          HttpResponse::Code::OK,
          std::string(std::istreambuf_iterator<char>{fstrm}, std::istreambuf_iterator<char>{}));
    } else {
      return HttpResponse::notFound(HttpRequest::Method::GET, url_path);
    }
  }

  static std::unique_ptr<HttpHandler> fileFetcher(const std::string& url_path) {
    return fromFunction([url_path](const HttpRequest&) { return loadFile(url_path); });
  }

  virtual ~HttpHandler(){};
  virtual HttpResponse handle_request(const HttpRequest& request) = 0;
};

template <typename F>
class Lambda : public HttpHandler, F {
public:
  Lambda(const F& lambda) noexcept : F(lambda) {}
  Lambda(F&& lambda) noexcept : F(std::move(lambda)) {}

  HttpResponse handle_request(const HttpRequest& request) override {
    return F::operator()(request);
  }
};

template <typename F>
  requires std::convertible_to<std::result_of_t<F(const HttpRequest&)>, HttpResponse>
std::unique_ptr<HttpHandler> HttpHandler::fromFunction(F&& lambda) {
  return std::make_unique<Lambda<F>>(std::forward<F>(lambda));
}


} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_HANDLER_HPP
