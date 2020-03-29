#include "network.h"
#include <iostream>
#include <string.h>
#include <assert.h>

TCPConn::TCPConn(int socket, size_t buf_len) : socket_(socket), buf_len_(buf_len) {
  struct sockaddr_in6 clientaddr;
  unsigned int addrlen = sizeof(clientaddr);
  getpeername(socket_, (struct sockaddr *)&clientaddr, &addrlen);
  char str[INET6_ADDRSTRLEN];
  if (inet_ntop(AF_INET6, &clientaddr.sin6_addr, str, sizeof(str))) {
    addr_ = str;
    port_ = ntohs(clientaddr.sin6_port);
  }
  if (setsockopt(socket_, SOL_SOCKET, SO_RCVLOWAT, (char *)&buf_len_,
                 sizeof(buf_len_)) < 0) {
    perror("setsockopt(SO_RCVLOWAT) failed");
    return;
  }
}

const std::string& TCPConn::address() const {
  return addr_;
}

int TCPConn::port() const {
  return port_;
}

void TCPConn::recv(void *out) {
  assert(socket_ != -1);
  int rc = ::recv(socket_, out, buf_len_, 0);
  if (rc < 0) {
    perror("recv() failed");
    return;
  }
  if (rc == 0 || rc < buf_len_) {
    printf("The client closed the connection before all of the\n");
    printf("data was sent\n");
  }
  for (auto i = 0; i < buf_len_; ++i) {
    std::cout << std::hex << ((char *)out)[i];
  }
  std::cout << "\n";
}

void TCPConn::send(void *in) {
  assert(socket_ != -1);
  int rc = ::send(socket_, in, buf_len_, 0);
  if (rc < 0) {
    perror("send() failed");
    return;
  }
}

TCPServer::TCPServer(int port) noexcept {
  int sd = -1;
  int on = 1;
  struct sockaddr_in6 serveraddr;

  if ((sd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    return;
  }

  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return;
  }

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin6_family = AF_INET6;
  serveraddr.sin6_port = htons(port);

  if (bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    perror("bind() failed");
    return;
  }

  socket_ = sd;
}

TCPConn TCPServer::listen() {
  assert(socket_ != -1);
  int conn = -1;
  if (::listen(socket_, 10) < 0) {
    perror("listen() failed");
    return TCPConn();
  }
  if ((conn = accept(socket_, NULL, NULL)) < 0) {
    perror("accept() failed");
    return TCPConn();
  }
  return TCPConn(conn, BUFFER_LENGTH);
}

TCPConn TCPConn::create(std::string server, size_t port, size_t buf_len) {
  int sd = -1;
  int rc;
  int rcdsize = buf_len;
  auto port_str = std::to_string(port);
  struct in6_addr serveraddr;
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0x00, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  rc = inet_pton(AF_INET, server.c_str(), &serveraddr);
  if (rc == 1) { /* valid IPv4 text address? */
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  } else {
    rc = inet_pton(AF_INET6, server.c_str(), &serveraddr);
    if (rc == 1) { /* valid IPv6 text address? */
      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_NUMERICHOST;
    }
  }
  rc = getaddrinfo(server.c_str(), port_str.c_str(), &hints, &res);
  if (rc != 0) {
    printf("Host not found --> %s\n", gai_strerror(rc));
    if (rc == EAI_SYSTEM) {
      perror("getaddrinfo() failed");
      return TCPConn();
    }
  }
  sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sd < 0) {
    perror("socket() failed");
    return TCPConn();
  }
  rc = connect(sd, res->ai_addr, res->ai_addrlen);
  if (rc < 0) {
    perror("connect() failed");
    return TCPConn();
  }
  return TCPConn(sd, buf_len);
}

#define MAX_UDP_LEN 1024 

UDPServer::UDPServer(int port) noexcept {
  int sd = -1;
  int on = 1;
  struct sockaddr_in6 serveraddr;

  if ((sd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    perror("socket() failed");
    return;
  }

  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return;
  }

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin6_family = AF_INET6;
  serveraddr.sin6_port = htons(port);

  if (bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    perror("bind() failed");
    return;
  }

  socket_ = sd;
}

UDPConn UDPServer::recv(void* out, size_t* len) {
  assert(socket_ != -1);
  struct sockaddr_in6 cliaddr; 
  memset(&cliaddr, 0, sizeof(cliaddr)); 

  unsigned int cliaddr_len = sizeof(cliaddr);  //len is value/resuslt 
  *len = recvfrom(socket_, (char *)out, BUFFER_LENGTH,
      0, ( struct sockaddr *) &cliaddr, 
      &cliaddr_len); 
  return UDPConn(socket_, cliaddr);
}

UDPConn::UDPConn(int socket, struct sockaddr_in6 sockaddr) : socket_(socket), sockaddr_(sockaddr) {
  assert(socket_ != 0);
  //unsigned int addrlen = sizeof(sockaddr_);
  //getpeername(socket_, (struct sockaddr *)&sockaddr_, &addrlen);
  char str[INET6_ADDRSTRLEN];
  port_ = ntohs(sockaddr_.sin6_port);
  if (inet_ntop(AF_INET6, &sockaddr_.sin6_addr, str, sizeof(str))) {
    addr_ = str;
  }
}

UDPConn UDPConn::create(std::string server, size_t port) {
  int sd = -1;
  int rc;
  auto port_str = std::to_string(port);
  struct sockaddr_in6 serveraddr;
  serveraddr.sin6_family = AF_INET6;
  serveraddr.sin6_port = htons(port);
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0x00, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  rc = inet_pton(AF_INET, server.c_str(), &serveraddr.sin6_addr);
  if (rc == 1) { /* valid IPv4 text address? */
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  } else {
    rc = inet_pton(AF_INET6, server.c_str(), &serveraddr.sin6_addr);
    if (rc == 1) { /* valid IPv6 text address? */
      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_NUMERICHOST;
    }
  }
  rc = getaddrinfo(server.c_str(), port_str.c_str(), &hints, &res);
  if (rc != 0) {
    printf("Host not found --> %s\n", gai_strerror(rc));
    if (rc == EAI_SYSTEM) {
      perror("getaddrinfo() failed");
      return UDPConn();
    }
  }
  sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  return UDPConn(sd, serveraddr);
}

void UDPConn::recv(void* out, size_t* len) {
  assert(socket_ != 0);
  unsigned int addr_size;
  *len = recvfrom(socket_, (char *)out, BUFFER_LENGTH,
      0, ( struct sockaddr *) &sockaddr_, 
      &addr_size);
}

void UDPConn::send(void* in, size_t len) {
  assert(socket_ != 0);
  sendto(socket_, (const char *)in, len, 
    0, (const struct sockaddr *) &sockaddr_,  
    sizeof(sockaddr_)); 
}


