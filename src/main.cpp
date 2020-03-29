#include "network.h"
#ifdef USE_OPENCV
#include "video.h"
#endif

#include <iostream>

#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  int port;
  if (argc == 3) {
    port = std::stoi(argv[2]);
#ifdef USE_OPENCV
  } else if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v') {
    try {
      Video v{};
      Curses c{};
      while (true) {
        c.checkInput();
        auto frm = v.frame();
        v.captureFrame(&frm);
        c.draw(frm, 5, 5, 80, 300);
        usleep(10000);
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
    }
    return 0;
#endif
  } else if (argc == 2) {
    port = std::stoi(argv[1]);
  } else {
    std::cerr << "Usage: p2pvcv2 [port] or p2pvcv2 [host] [port]\n Or -v for video.\n";
    return 1;
  }

  TCPServer tcp_server(port);
  UDPServer udp_server(port + 1);

  if (argc == 3) {
    //auto conn = TCPConn::create(argv[1], port, BUFFER_LENGTH);
    auto u_conn = UDPConn::create(argv[1], port + 1);
    //std::cout << "connected to " << conn.address() << ":" << conn.port() << "\n";
    char buf[BUFFER_LENGTH];
    memset(buf, 'a', BUFFER_LENGTH);
    u_conn.send((void*)buf, BUFFER_LENGTH);
    std::cout << "sent " << BUFFER_LENGTH << "\n";
    size_t res_len;
    u_conn.recv((void*)buf, &res_len);
    std::cout << "recvd " << res_len << "\n";
    return 0;
  }

  while (true) {
    char buf[BUFFER_LENGTH];
    size_t res_len = BUFFER_LENGTH;
    std::cout << "waiting to recv " << res_len << "\n";
    auto u_conn = udp_server.recv((void*)buf, &res_len);
    //u_conn.recv((void*)buf, &res_len);
    std::cout << "recvd " << res_len << "\n";
    for (auto i = 0; i < BUFFER_LENGTH; ++i) {
      buf[i]++;
    }
    u_conn.send((void*)buf, res_len);
    std::cout << "sent " << res_len << "\n";
    //std::cout << "new connection " << conn.address() << ":" << conn.port() << "\n";
  }

  return 0;
}
