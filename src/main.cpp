#include "network.h"

#include <iostream>

int main(int argc, char *argv[]) {
  // thread 1
  // spawn server
  // thread 2-N
  // connect to clients, set up UDP connections
  // thread N+
  int port;
  if (argc == 3) {
    port = std::stoi(argv[2]);
  } else if (argc == 2) {
    port = std::stoi(argv[1]);
  } else {
    std::cerr << "Usage: p2pvcv2 [port] or p2pvcv2 [host] [port]\n";
    return 1;
  }

  TCPServer tcp_server(port);

  if (argc == 3) {
    auto conn = TCPConn::create(argv[1], port, BUFFER_LENGTH);
    std::cout << "connected as host to " << conn.address() << ":" << conn.port() << "\n";
  }
  while (true) {
    auto conn = tcp_server.listen();
    std::cout << "new connection " << conn.address() << ":" << conn.port() << "\n";
  }
  return 0;
}
