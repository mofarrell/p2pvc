#include <stddef.h>
#include <string>

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

