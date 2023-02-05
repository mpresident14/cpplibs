#include "src/webserver/http_server.hpp"

#include "http_server.hpp"
#include "src/webserver/http_structures.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <boost/container_hash/hash.hpp>

#include <netinet/in.h>
#include <sys/socket.h>


namespace prez::webserver {
namespace {


int check_err(int result, const char* msg) {
  if (result < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
  return result;
}


} // namespace


HttpResponse HttpServer::process_request(char* buffer) const {
  try {
    HttpRequest request = HttpRequest::parse(buffer);
    std::cout << "Received:\n" << request << std::endl;

    auto handler = handlers_.find({static_cast<int>(request.method_), request.path_});
    if (handler == handlers_.end()) {
      return HttpResponse(
          HttpResponse::Code::NOT_FOUND,
          std::string(HttpRequest::METHODS.at(request.method_))
              .append(1, ' ')
              .append(request.path_)
              .append(" was not found on this server."));
    } else {
      return handler->second->handle_request(request);
    }
  } catch (const std::invalid_argument& e) {
    // TODO: Use custom exception type so we don't catch things from handler.
    return HttpResponse(HttpResponse::Code::BAD_REQUEST, e.what());
  } catch (const std::exception& e) {
    return HttpResponse(HttpResponse::Code::INTERNAL, e.what());
  }
}


void HttpServer::handle_requests(size_t max_queued_connections, u_int16_t port) const {
  // Create the TPC socket
  int sd = check_err(socket(AF_INET, SOCK_STREAM, 0), "socket()");

  // Allow immediate reuse of address and port
  int optval = 1;
  check_err(
      setsockopt(sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)),
      "setsockopt()");

  // Bind socket to this port on any IP addres
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  check_err(bind(sd, (struct sockaddr*)&addr, addr_len), "bind()");

  // Prepare socket to accept connections
  check_err(listen(sd, max_queued_connections), "listen()");

  // Respond to requests
  while (true) {
    int new_socket = check_err(accept(sd, (struct sockaddr*)&addr, &addr_len), "accept()");
    char buffer[BUFLEN] = {0};
    check_err(read(new_socket, buffer, BUFLEN), "read()");

    std::stringstream response;
    response << process_request(buffer);
    std::string response_str = response.str();
    std::cout << "Sending:\n" << response_str << std::endl;
    check_err(send(new_socket, response_str.data(), response_str.length(), 0), "send()");

    close(new_socket);
  }

  close(sd);
}

void HttpServer::run(size_t num_threads, size_t max_queued_connections, u_int16_t port) const {
  std::cout << "Spawning " << num_threads << " threads" << std::endl;
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back(
        [this](size_t arg_max_queued_connections, u_int16_t arg_port) {
          this->handle_requests(arg_max_queued_connections, arg_port);
        },
        max_queued_connections,
        port);
  }
  for (std::thread& thread : threads) {
    thread.join();
  }
}

void prez::webserver::HttpServer::install_handler(
    HttpRequest::Method method, std::string path, std::unique_ptr<HttpHandler> handler) {
  handlers_.emplace(std::make_pair(static_cast<int>(method), path), std::move(handler));
}


} // namespace prez::webserver
