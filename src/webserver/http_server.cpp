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


namespace prez::webserver {
namespace {

constexpr size_t PORT = 8080;
constexpr size_t BUFLEN = 1024;
constexpr size_t MAX_CONNECTIONS_QUEUED = 5;
constexpr size_t NUM_THREADS = 2;

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

prez::webserver::HttpResponse process_request(char* buffer) {
  try {
    prez::webserver::HttpRequest request = prez::webserver::HttpRequest::parse(buffer);
    std::cout << "Method : " << request.method_ << std::endl;
    std::cout << "Path : " << request.path_ << std::endl;
    std::cout << "Query Params : " << request.query_params_ << std::endl;
    std::cout << "Version : " << request.version_ << std::endl;
    std::cout << "Headers : " << prez::misc::OStreamable(request.headers_) << std::endl;
  } catch (const std::invalid_argument& e) {
    return prez::webserver::HttpResponse(prez::webserver::HttpCode::BAD_REQUEST, e.what());
  } catch (const std::exception& e) {
    return prez::webserver::HttpResponse(prez::webserver::HttpCode::INTERNAL, e.what());
  }

  return prez::webserver::HttpResponse(prez::webserver::HttpCode::OK, REPSONSE_BODY);
}

int handle_requests() {
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
  addr.sin_port = htons(PORT);
  check_err(bind(sd, (struct sockaddr*)&addr, addr_len), "bind()");

  // Prepare socket to accept connections
  check_err(listen(sd, MAX_CONNECTIONS_QUEUED), "listen()");

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
} // namespace

void run_server() {
  std::cout << "Spawning " << NUM_THREADS << " threads" << std::endl;
  std::vector<std::thread> threads;
  threads.reserve(NUM_THREADS);
  for (size_t i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(handle_requests);
  }
  for (std::thread& thread : threads) {
    thread.join();
  }
}
} // namespace prez::webserver
