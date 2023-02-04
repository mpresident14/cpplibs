#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFLEN 1024
#define MAX_CONNECTIONS 3
#define NUM_THREADS 3

int check_err(int result, const char* msg) {
  if (result < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
  return result;
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
  check_err(listen(sd, MAX_CONNECTIONS), "listen()");

  // Respond to requests
  while (true) {
    int new_socket = check_err(accept(sd, (struct sockaddr*)&addr, &addr_len), "accept()");
    char buffer[BUFLEN] = {0};
    read(new_socket, buffer, BUFLEN);
    std::cout << "Received " << buffer << " on thread " << std::this_thread::get_id() << std::endl;

    std::stringstream response;
    response << "Hello from thread " << std::this_thread::get_id();
    std::string response_str = response.str();
    send(new_socket, response_str.data(), response_str.length(), 0);

    close(new_socket);
  }

  close(sd);
}

int main() {
  std::cout << "Spawning " << NUM_THREADS << " threads" << std::endl;
  std::vector<std::thread> threads;
  threads.reserve(NUM_THREADS);
  for (size_t i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back(handle_requests);
  }
  for (std::thread& thread : threads) {
    thread.join();
  }

  return 0;
}
