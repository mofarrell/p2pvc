#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <p2plib.h>
#include <unistd.h>

static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;

void callback(connection_t *con, void *data, size_t datalen) {
  printf("DATA: %s\n", (char *)data);
}

void new_callback(connection_t *con, void *data, size_t datalen) {
  printf("newDATA: %s\n", (char *)data);
  pthread_mutex_lock(&conslock);
  conslen++;
  printf("NEW PERSONSDFSDFSDF %d\n", (int)conslen);
  cons = realloc(cons, conslen * sizeof(connection_t));
  printf("HISDF");
  memcpy(&(cons[conslen-1]), con, sizeof(connection_t));
  pthread_mutex_unlock(&conslock);
}

void *dolisten(void *args) {
  int socket;
  p2p_init(55555, &socket);
  p2p_listener((connection_t **)&cons, &conslen, &conslock, &callback, &new_callback, socket);
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: p2pvc [server] [port]\n");
  }

  cons = calloc(1, sizeof(connection_t));
  if (!p2p_connect(argv[1], argv[2], &(cons[0]))) {
    conslen++;
  }

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, NULL);

  while(1){
    printf("sending\n");
    char buf[10] = "HELLO";
    p2p_broadcast(&cons, &conslen, &conslock, buf, 7); 
    sleep(1);
  }

  return 0;
}

