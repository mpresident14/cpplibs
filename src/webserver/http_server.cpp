#include "src/webserver/http_server.hpp"

#include "src/misc/ostreamable.hpp"
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

#include <netinet/in.h>
#include <sys/socket.h>
#include "http_server.hpp"


namespace prez::webserver {
namespace {

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
    std::cout << request << std::endl;
  } catch (const std::invalid_argument& e) {
    return HttpResponse(HttpResponse::Code::BAD_REQUEST, e.what());
  } catch (const std::exception& e) {
    return HttpResponse(HttpResponse::Code::INTERNAL, e.what());
  }

  return HttpResponse(HttpResponse::Code::OK, REPSONSE_BODY);
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
    std::cout << "Received:\n"
              << buffer << " on thread " << std::this_thread::get_id() << std::endl;

    std::stringstream response;
    response << process_request(buffer);
    std::string response_str = response.str();
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

void prez::webserver::HttpServer::install_handler() {}


} // namespace prez::webserver
