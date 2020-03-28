#include "network.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * 1) Initialize a persistent TCP server
 * 2) Initialize a pool of UDP connections
 * 2) Optionally connect to client TCP servers
 */
#define BUFFER_LENGTH 256
#define SERVER_PORT 3002
#define SERVER_NAME     "localhost"
#define MAX_HOST_NAME_LENGTH 256
TCPServer::TCPServer() noexcept {
  int sd=-1;
  int sdconn=-1;
  int rc;
  int on=1;
  int rcdsize=BUFFER_LENGTH;
  char buffer[BUFFER_LENGTH];
  struct sockaddr_in6 serveraddr , clientaddr;
  unsigned int addrlen=sizeof(clientaddr);
  char str[INET6_ADDRSTRLEN];

  if ((sd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    return;
  }

  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&on,sizeof(on)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return;
  }

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin6_family = AF_INET6;
  serveraddr.sin6_port   = htons(SERVER_PORT);

  if (bind(sd,
        (struct sockaddr *)&serveraddr,
        sizeof(serveraddr)) < 0) {
    perror("bind() failed");
    return;
  }
  if (listen(sd, 10) < 0) {
    perror("listen() failed");
    return;
  }
  std::cerr << "server started\n";
  if ((sdconn = accept(sd, NULL, NULL)) < 0) {
    perror("accept() failed");
    return;
  } else {
    getpeername(sdconn, (struct sockaddr *)&clientaddr, &addrlen);
    if(inet_ntop(AF_INET6, &clientaddr.sin6_addr, str, sizeof(str))) {
      printf("Client address is %s\n", str);
      printf("Client port is %d\n", ntohs(clientaddr.sin6_port));
    }
  }
  if (setsockopt(sdconn, SOL_SOCKET, SO_RCVLOWAT,
        (char *)&rcdsize,sizeof(rcdsize)) < 0) {
    perror("setsockopt(SO_RCVLOWAT) failed");
    return;
  }
  rc = recv(sdconn, buffer, sizeof(buffer), 0);
  if (rc < 0) {
    perror("recv() failed");
    return;
  }
  printf("%d bytes of data were received\n", rc);
  if (rc == 0 || rc < sizeof(buffer)) {
    printf("The client closed the connection before all of the\n");
    printf("data was sent\n");
    return;
  }
  rc = send(sdconn, buffer, sizeof(buffer), 0);
  if (rc < 0) {
    perror("send() failed");
    return;
  }
}

TCPClient::TCPClient() noexcept {
  int    sd=-1, rc, bytesReceived=0;
  int rcdsize=BUFFER_LENGTH;
  char   buffer[BUFFER_LENGTH];
  char   server[MAX_HOST_NAME_LENGTH];
  char   servport[] = "3002";
  struct in6_addr serveraddr;
  struct addrinfo hints, *res=NULL;
  strcpy(server, SERVER_NAME);
  memset(&hints, 0x00, sizeof(hints));
  hints.ai_flags    = AI_NUMERICSERV;
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  rc = inet_pton(AF_INET, server, &serveraddr);
  if (rc == 1)    /* valid IPv4 text address? */
  {
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  }
  else
  {
    rc = inet_pton(AF_INET6, server, &serveraddr);
    if (rc == 1) /* valid IPv6 text address? */
    {

      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_NUMERICHOST;
    }
  }
  rc = getaddrinfo(server, servport, &hints, &res);
  if (rc != 0)
  {
    printf("Host not found --> %s\n", gai_strerror(rc));
    if (rc == EAI_SYSTEM)
      perror("getaddrinfo() failed");
    return;
  }
  sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sd < 0) {
    perror("socket() failed");
    return;
  }
  rc = connect(sd, res->ai_addr, res->ai_addrlen);
  if (rc < 0) {
    perror("connect() failed");
    return;
  }
  if (setsockopt(sd, SOL_SOCKET, SO_RCVLOWAT,
        (char *)&rcdsize,sizeof(rcdsize)) < 0) {
    perror("setsockopt(SO_RCVLOWAT) failed");
    return;
  }
  memset(buffer, 'a', sizeof(buffer));
  rc = send(sd, buffer, sizeof(buffer), 0);
  if (rc < 0) {
    perror("send() failed");
    return;
  }
  printf("done\n");
}

//struct UDPConnection {
//  void send(void* data, size_t len);
//  void recv(void** data, size_t* len);
//};
//
//struct Peer {
//  TCPConnection tcp;
//  UDPConnection udp;
//};
//
//struct Server {
//  const std::vector<const Peer*>& peers() {
//    return {};
//  }
//
//  void broadcast(void* data, size_t len) {
//    for (auto& peer : peers()) {
//      peer.udp.send(data, len);
//    }
//  }
//};
//
