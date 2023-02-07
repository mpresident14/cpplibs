#include "src/webserver/http_server.hpp"
#include "src/webserver/http_structures.hpp"
#include "src/webserver/json/json_parser.hpp"

#include <fstream>

namespace pweb = prez::webserver;

int main() {
  pweb::HttpServer server;

  server.install_handler(
      pweb::HttpRequest::Method::GET,
      "/",
      pweb::HttpHandler::fileFetcher("/src/webserver/testfiles/index.html"));

  server.install_handler(
      pweb::HttpRequest::Method::POST,
      "/echo",
      pweb::HttpJsonHandler::fromFunction(
          [](const pweb::HttpRequest&, const pweb::json::JsonObject& json) {
            try {
              return pweb::HttpResponse(pweb::HttpResponse::Code::OK, json.get_string("echo"));
            } catch (const std::invalid_argument& e) {
              return pweb::HttpResponse(pweb::HttpResponse::Code::BAD_REQUEST, e.what());
            } catch (const std::out_of_range& e) {
              return pweb::HttpResponse(pweb::HttpResponse::Code::BAD_REQUEST, e.what());
            }
          }));

  server.run(2, 5, 8080);
}
