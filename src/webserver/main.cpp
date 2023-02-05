#include "src/webserver/http_server.hpp"
#include "src/webserver/http_structures.hpp"


int main() {
  prez::webserver::HttpServer server;
  server.install_handler(
      prez::webserver::HttpRequest::Method::GET,
      "/",
      prez::webserver::HttpHandler::fileFetcher("/src/webserver/testfiles/index.html"));
  server.run(2, 5, 8080);
}
