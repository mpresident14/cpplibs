#include "src/webserver/http_server.hpp"
#include "src/webserver/http_structures.hpp"
#include "src/webserver/json/json_parser.hpp"

#include <fstream>

// int main() {
//   prez::webserver::HttpServer server;
//   server.install_handler(
//       prez::webserver::HttpRequest::Method::GET,
//       "/",
//       prez::webserver::HttpHandler::fileFetcher("/src/webserver/testfiles/index.html"));
//   server.run(2, 5, 8080);
// }

int main() {
  std::ifstream fstrm("src/webserver/testfiles/test.json");
  if (!fstrm.is_open()) {
    throw std::invalid_argument("not open");
  }
  std::string json_str =
      std::string(std::istreambuf_iterator<char>{fstrm}, std::istreambuf_iterator<char>{});
  auto json = json_parser::parseString(json_str);
  std::cout << *json << std::endl;
}
