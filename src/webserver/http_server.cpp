#include "src/webserver/http_server.hpp"

#include "http_server.hpp"
#include "src/webserver/http_handler.hpp"
#include "src/webserver/http_structures.hpp"

#include <deque>
#include <future>
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

using namespace std::chrono_literals;


HttpResponse HttpServer::process_request(const char* buffer) const {
  try {
    HttpRequest request = HttpRequest::parse(buffer);
    std::cout << "Received:\n" << request << std::endl;

    auto handler = handlers_.find({static_cast<int>(request.method_), request.path_});
    if (handler != handlers_.end()) {
      return handler->second->handle_request(request);
    }
    // Default for GET is to read file
    else if (request.method_ == HttpRequest::Method::GET) {
      return HttpHandler::loadFile(request.path_);
    }

    return HttpResponse::notFound(request.method_, request.path_);

  } catch (const std::invalid_argument& e) {
    // TODO: Use custom exception type so we don't catch things from handler.
    return HttpResponse(HttpResponse::Code::BAD_REQUEST, e.what());
  } catch (const std::exception& e) {
    return HttpResponse(HttpResponse::Code::INTERNAL, e.what());
  }
}

void HttpServer::handle_request(int client_sd, const char* buffer) const {
  std::stringstream response;
  response << process_request(buffer);
  std::string response_str = response.str();
  std::cout << "Sending:\n" << response_str << std::endl;
  check_err(send(client_sd, response_str.data(), response_str.length(), 0), "send()");
  close(client_sd);
}

void HttpServer::run(size_t num_threads, size_t max_queued_connections, u_int16_t port) const {
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
  std::cout << "Initializing threadpool with " << num_threads << " threads" << std::endl;
  std::deque<std::future<void>> threadpool;

  while (true) {
    int new_socket = check_err(accept(sd, (struct sockaddr*)&addr, &addr_len), "accept()");
    char buffer[BUFLEN] = {0};
    check_err(read(new_socket, buffer, BUFLEN), "read()");

    // When threadpool is full, reallocate. Should probably do this incrementally, not "stop the
    // world", but oh well.
    while (threadpool.size() >= num_threads) {
      std::cout << "Threadpool is full" << std::endl;
      std::erase_if(threadpool, [](const auto& future) {
        return future.wait_for(0ms) == std::future_status::ready;
      });
    }

    // Delegate the work to a new thread.
    std::packaged_task<void()> response_task(
        [new_socket, buffer, this] { return handle_request(new_socket, buffer); });
    threadpool.push_back(response_task.get_future());
    std::thread(std::move(response_task)).detach();
  }
}

void prez::webserver::HttpServer::install_handler(
    HttpRequest::Method method, std::string path, std::unique_ptr<HttpHandler> handler) {
  handlers_.emplace(std::make_pair(static_cast<int>(method), path), std::move(handler));
}


} // namespace prez::webserver
