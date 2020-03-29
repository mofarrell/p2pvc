#pragma once

#include <stddef.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_LENGTH 256

struct TCPConn {
  TCPConn() = default;
  TCPConn(int socket, size_t buf_len);
  static TCPConn create(std::string server, size_t port, size_t buf_len);

  void recv(void *out);
  void send(void *in);

  int port() const;
  const std::string& address() const;

 private:
  int socket_ = -1;
  size_t buf_len_;
  std::string addr_;
  int port_ = -1;
};

struct TCPServer {
  TCPServer(int port) noexcept;
  TCPConn listen();
 private:
  int socket_ = -1;
};

struct UDPConn {
  UDPConn() = default;
  UDPConn(int socket, struct sockaddr_in6 sockaddr);
  static UDPConn create(std::string server, size_t port);
 private:
  int socket_ = -1;
  struct sockaddr_in6 sockaddr_;
  std::string addr_;
  int port_ = -1;
};

struct UDPServer {
  UDPServer(int port) noexcept;
  UDPConn listen();
 private:
  int socket_ = -1;
};
