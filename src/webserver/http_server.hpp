#ifndef PREZ_WEBSERVER_HTTP_SERVER_HPP
#define PREZ_WEBSERVER_HTTP_SERVER_HPP

#include "src/webserver/http_handler.hpp"
#include "src/webserver/http_structures.hpp"

#include <cstddef>
#include <unordered_map>
#include <utility>

#include <boost/container_hash/hash.hpp>

namespace prez::webserver {

class HttpServer {
public:
  void run(size_t num_threads, size_t max_queued_connections, u_int16_t port) const;
  void install_handler(
      HttpRequest::Method method,
      std::string path,
      // Owned by HttpServer
      std::unique_ptr<HttpHandler> handler);

private:
  static constexpr size_t BUFLEN = 1024;

  void handle_requests(size_t max_queued_connections, u_int16_t port) const;
  HttpResponse process_request(char* buffer) const;

  std::unordered_map<
      std::pair<int, std::string>,
      std::unique_ptr<HttpHandler>,
      boost::hash<std::pair<int, std::string>>>
      handlers_;
};

} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_SERVER_HPP
