#ifndef PREZ_WEBSERVER_HTTP_HANDLER_HPP
#define PREZ_WEBSERVER_HTTP_HANDLER_HPP

#include "src/webserver/http_structures.hpp"
#include "src/webserver/json/json.hpp"
#include "src/webserver/json/json_parser.hpp"

#include <concepts>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>
#include <filesystem>

namespace prez::webserver {


class HttpHandler {
public:
  template <typename F>
    requires std::convertible_to<std::result_of_t<F(const HttpRequest&)>, HttpResponse>
  static std::unique_ptr<HttpHandler> fromFunction(F&& lambda);

  static HttpResponse loadFile(std::string_view url_path) {
    // TODO: Make this secure: no absolute paths, dots, etc.
    // TODO: assert len(url_path) > 0
    // Skip the initial / in the URL path.
    std::ifstream fstrm(url_path.data() + 1);
    if (fstrm.is_open()) {
      return HttpResponse(
          HttpResponse::Code::OK,
          std::string(std::istreambuf_iterator<char>{fstrm}, std::istreambuf_iterator<char>{}));
    } else {
      std::cerr << strerror(errno) << std::endl;
      return HttpResponse::notFound(HttpRequest::Method::GET, url_path);
    }
  }

  static std::unique_ptr<HttpHandler> fileFetcher(std::string_view url_path) {
    return fromFunction([url_path](const HttpRequest&) { return loadFile(url_path); });
  }

  virtual ~HttpHandler(){};
  virtual HttpResponse handle_request(const HttpRequest& request) = 0;
};

class HttpJsonHandler : public HttpHandler {
public:
  template <typename F>
    requires std::convertible_to<
        std::result_of_t<F(const HttpRequest&, const json::JsonObject& json)>,
        HttpResponse>
  static std::unique_ptr<HttpJsonHandler> fromFunction(F&& lambda);

  HttpResponse handle_request(const HttpRequest& request) override {
    try {
      auto json = json_parser::parseString(request.body_);
      return handle_json(request, *json);
    } catch (const json_parser::ParseException& e) {
      std::cerr << e.what() << std::endl;
      return HttpResponse(HttpResponse::Code::BAD_REQUEST, "Invalid JSON");
    }
  }

protected:
  virtual HttpResponse handle_json(const HttpRequest& request, const json::JsonObject& json) = 0;
};

namespace {

template <typename F>
class HttpLambda : public HttpHandler, F {
public:
  HttpLambda(const F& lambda) noexcept : F(lambda) {}
  HttpLambda(F&& lambda) noexcept : F(std::move(lambda)) {}

  HttpResponse handle_request(const HttpRequest& request) override {
    return F::operator()(request);
  }
};

template <typename F>
class HttpJsonLambda : public HttpJsonHandler, F {
public:
  HttpJsonLambda(const F& lambda) noexcept : F(lambda) {}
  HttpJsonLambda(F&& lambda) noexcept : F(std::move(lambda)) {}

  HttpResponse handle_json(const HttpRequest& request, const json::JsonObject& json) override {
    return F::operator()(request, json);
  }
};

} // namespace

template <typename F>
  requires std::convertible_to<std::result_of_t<F(const HttpRequest&)>, HttpResponse>
std::unique_ptr<HttpHandler> HttpHandler::fromFunction(F&& lambda) {
  return std::make_unique<HttpLambda<F>>(std::forward<F>(lambda));
}

template <typename F>
  requires std::convertible_to<
      std::result_of_t<F(const HttpRequest&, const json::JsonObject& json)>,
      HttpResponse>
std::unique_ptr<HttpJsonHandler> HttpJsonHandler::fromFunction(F&& lambda) {
  return std::make_unique<HttpJsonLambda<F>>(std::forward<F>(lambda));
}

} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_HANDLER_HPP
