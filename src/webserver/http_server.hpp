#ifndef PREZ_WEBSERVER_HTTP_SERVER_HPP
#define PREZ_WEBSERVER_HTTP_SERVER_HPP

#include "src/webserver/http_structures.hpp"

#include <cstddef>

namespace prez::webserver {

class HttpServer {
public:
  void run(size_t num_threads, size_t max_queued_connections, u_int16_t port) const;
  void install_handler();

private:
  constexpr static size_t BUFLEN = 1024;

  void handle_requests(size_t max_queued_connections, u_int16_t port) const;
  HttpResponse process_request(char* buffer) const;
};

} // namespace prez::webserver

#endif // PREZ_WEBSERVER_HTTP_SERVER_HPP
