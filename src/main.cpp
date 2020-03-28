#include <iostream>
#include "network.h"

int main(int argc, char *argv[]) {
  // thread 1
  // spawn server
  // thread 2-N
  // connect to clients, set up UDP connections
  // thread N+
  if (argc > 1) {
    TCPServer t;
  } else {
    TCPClient t;
  }
  std::cout << "hello world!\n";
}
