/* @file p2plib.c
 * @brief Implements p2plib.
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

#define MAX_PACKET_SIZE   4096
#define UDP_FLAGS         0

/* @brief A status for connections. */
typedef struct {
  int socket;
  struct sockaddr_in addr;
  socklen_t addr_len;
} connection_t;

int create_client(char *server_name, char *server_port, connection_t *c) {
  struct addrinfo protocol_spec;
  struct addrinfo *possible_addrs, *curr_addr;
  int err = 0;
  memset(&protocol_spec, 0, sizeof(struct addrinfo));

  protocol_spec.ai_family = AF_INET;
  protocol_spec.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  protocol_spec.ai_flags = AI_PASSIVE; /* For wildcard IP address */
  protocol_spec.ai_protocol = IPPROTO_UDP; /* XXX: UDP protocol */
  protocol_spec.ai_canonname = NULL;
  protocol_spec.ai_addr = NULL;
  protocol_spec.ai_next = NULL;

  if ((err = getaddrinfo(server_name, server_port,
       &protocol_spec, &possible_addrs)) != 0) {
    fprintf(stderr, "error in getaddrinfo %s\n", strerror(errno));
    return (err);
  }

  curr_addr = possible_addrs;
  while (curr_addr != NULL) {
    c->socket = socket(curr_addr->ai_family, curr_addr->ai_socktype, curr_addr->ai_protocol);
    if (c->socket > 0) {
      /* XXX : bind to server */
      break;
    }
    curr_addr = curr_addr->ai_next;
  }

  if (curr_addr = NULL) {
    fprintf(stderr, "unable to find a server in create_client\n");
    freeaddrinfo(possible_addrs);
    return (ENOTCONN);
  }

  c->addr.sin_family = curr_addr->ai_family;
  c->addr.sin_port = htons(PORT);

  if (curr_addr->ai_family == AF_INET) {
     memcpy((void *)&con->addr.sin_addr, &((struct sockaddr_in *)curr_addr->ai_addr)->sin_addr, curr_addr->ai_addrlen);
  } else {
     fprintf(stderr, "Unable to use ai_family returned.");
    freeaddrinfo(possible_addrs);
     return (EINVAL);
  }
  
  freeaddrinfo(possible_addrs);
  c->addr_len = sizeof(con->addr);

  return (0);
}

/* @Brief create a server that can cold up to max_connections 
   @Return an errno or 0 on success*/
int p2p_init(int *sockfd) {
  int _sockfd_local;
  short port;
  struct sockaddr_in me;

  /* create socket */
  if ((_sockfd_local = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    fprintf(stderr, "error in opening socket: %s\n", strerror(errno));
    return (errno);
  }

  /* If caller wants, populate socket fd */

  if (sockfd != NULL) {
    *sockfd = _sockfd_local;
  }

  /* XXX: Make sure these fields are OK, possibly switch to variables.*/
  memset(&me, 0, sizeof(me));
  me.sin_family = AF_INET;
  me.sin_port = htons(PORT);
  me.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(_sockfd_local, &me, sizeof(me)) == -1) {
    fprintf(stderr, "error in binding socket: %s\n", strerror(errno));
    return (errno);
  }

  /* success */
  return (0);
}


/* @brief Initialize a listener.
 * @param cons A reference to a connection array.
 * @param conslen A reference to the length of the connection array.
 * @param consmutex A mutex to access the connection array.
 * Note: The reason there are references is so the array can be updated.
 *       In the case of a new connection, new_callback will be called.
 * @param callback The standard callback, passing in the data and the
 *        connection associated with the data.
 * @param new_callback Called when a new connection is discovered.
 *        If null nothing happens.
 * @return Negative value on error.
 */
int p2p_listener(connection_t **cons, size_t *conslen,
                 pthread_mutex_t *consmutex,
                 void (*callback)(connection_t *, void *, size_t),
                 void (*new_callback)(connection_t *, void *, size_t)) {

  /* A stack allocated connection struct to store any data
     about the connection we recieve. */
  connection_t con;
  char buf[MAX_PACKET_SIZE];

  /* Loop on recvfrom. */
  while (1) {
    memset(buf, 0, MAX_PACKET_SIZE);
    size_t recv_len = recvfrom(con.socket, buf, MAX_PACKET_SIZE, UDP_FLAGS, (struct sockaddr *)&(con.addr), &(con.addr_len));

    /* Handle error UDP style (try again). */
    if (recv_len < 0) {
      fprintf(stderr, "Recieve failed.\n");
      continue;
    }

    if(consmutex) {
      pthread_mutex_lock(consmutex);
    }

    /* Check if the connection we recieved from is in our array. */
    int i, new_connection = 1;
    for (i = 0; i < *conslen; i++) {
      if (con.addr.sin_addr.s_addr == (*cons)[i].addr.sin_addr.s_addr) {
        new_connection = 0;
        break;
      }
    }

    if(consmutex) {
      pthread_mutex_unlock(consmutex);
    }

    /* Now invoke callbacks. */
    if (new_connection) {
      /* Ignore new_callback if not defined. */
      if (new_callback) {
        (*new_callback)(&con, buf, recv_len);
      }
    } else {
      (*callback)(&con, buf, recv_len);
    }

  }

  return -1;
}

