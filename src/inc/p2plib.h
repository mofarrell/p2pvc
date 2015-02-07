/* @file inc/p2plib.h
 * @brief Defines the interface of p2plib.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

/* @brief A status for connections. */
typedef struct {
  int socket;
  struct sockaddr_in addr;
  socklen_t addr_len;
} connection_t;

int p2p_connect(char *server, char *port, connection_t *con);

int p2p_send(connection_t *con, void *buf, size_t buflen);

int p2p_broadcast(connection_t **cons, size_t *conslen, pthread_mutex_t *consmutex, void *buf, size_t buflen);

int p2p_listener(connection_t **cons, size_t *conslen,
                 pthread_mutex_t *consmutex,
                 void (*callback)(connection_t *, void *, size_t),
                 void (*new_callback)(connection_t *, void *, size_t),
                 int socket);

int p2p_init(int port, int *sockfd);

