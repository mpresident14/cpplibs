#include "src/webserver/http_server.hpp"
#include "src/webserver/http_structures.hpp"

constexpr char REPSONSE_BODY[] = "<!DOCTYPE html>"
                                 "<html lang='en'>"
                                 "  <head>"
                                 "    <title>CS142 Project #1</title>"
                                 "    <link rel='stylesheet' href='./styleC.css' />"
                                 "  </head>"
                                 "  <body>"
                                 "    <div class='container'>"
                                 "      <div class='box' id='A'>A</div>"
                                 "      <div class='box' id='B'>B</div>"
                                 "      <div class='box' id='C'>C</div>"
                                 "      <div class='box' id='D'>D</div>"
                                 "      <div class='box' id='E'>E</div>"
                                 "      <div class='box' id='F'>F</div>"
                                 "    </div>"
                                 "  </body>"
                                 "</html>";

int main() {
  prez::webserver::HttpServer server;
  server.install_handler(
      prez::webserver::HttpRequest::Method::GET,
      "/",
      prez::webserver::HttpHandler::fromLambda([](prez::webserver::HttpRequest) {
        return prez::webserver::HttpResponse(
            prez::webserver::HttpResponse::Code::OK, REPSONSE_BODY);
      }));
  server.run(2, 5, 8080);
}
