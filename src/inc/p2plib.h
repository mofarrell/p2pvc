/* @file inc/p2plib.h
 * @brief Defines the interface of p2plib.
 */

#ifndef P2PLIB_H
#define P2PLIB_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define P2P_HEADER 0x17381939

/* @brief A status for connections. */
typedef struct {
  int socket;
  struct sockaddr_in addr;
  socklen_t addr_len;
} connection_t;

/* @brief a type to signify what the p2p data should do */
typedef enum {
  PASS_HEADER,
  CONS_HEADER,
} p2p_action_t;

/* @brief A header for non-user interaction */
typedef struct {
  unsigned int check;
  p2p_action_t act;
} p2p_header_t;

long p2p_bandwidth(size_t packetsize);

int p2p_send_pass(connection_t *con, char *password);

int p2p_data(connection_t *con, void *data, size_t datalen, connection_t **cons, size_t *conlen);

int p2p_connect(char *server, char *port, connection_t *con);

int p2p_send(connection_t *con, const void *buf, size_t buflen);

int p2p_broadcast(connection_t **cons, size_t *conslen, pthread_mutex_t *consmutex, const void *buf, size_t buflen);

int p2p_listener(connection_t **cons, size_t *conslen,
                 pthread_mutex_t *consmutex,
                 void (*callback)(connection_t *, void *, size_t),
                 void (*new_callback)(connection_t *, void *, size_t),
                 int socket,
                 unsigned long max_packet_size);

int p2p_init(int port, int *sockfd);

#endif /* P2PLIB_H */
