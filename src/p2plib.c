/* @file p2plib.c
 * @brief Implements p2plib.
 */
#include <stdio.h>
#include <string.h>
#include <p2plib.h>

#define MAX_PACKET_SIZE   4096
#define UDP_FLAGS         0


int p2p_connect();

/* @brief Send data to a connection.
 * @param con The connection to send the data to.
 * @param buf The data to send.
 * @param buflen The length of the data to send.
 * @return Negative value on error, 0 on success.
 */
int p2p_send(connection_t *con, void *buf, size_t buflen) {
  return sendto(con->socket, buf, buflen, UDP_FLAGS, (struct sockaddr *)&con->addr, con->addr_len);
}

/* @brief Send data to all connections.
 * @param cons A reference to a connection array.
 * @param conslen A reference to the length of the connection array.
 * @param consmutex A mutex to access the connection array.
 * @param buf The data to send.
 * @param buflen The length of the data to send.
 * @return Negative value on error, 0 on success.
 */
int p2p_broadcast(connection_t **cons, size_t *conslen, pthread_mutex_t *consmutex, void *buf, size_t buflen) {
  if(consmutex) {
    pthread_mutex_lock(consmutex);
  }

  int i;
  for (i = 0; i < *conslen; i++) {
    p2p_send(&((*cons)[i]), buf, buflen);
  }

  if(consmutex) {
    pthread_mutex_unlock(consmutex);
  }
  return 0;
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

