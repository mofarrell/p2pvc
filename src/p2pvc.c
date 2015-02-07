#include <stdio.h>
#include <p2plib.h>

void callback(connection_t *con, void *data, size_t datalen) {
  printf("DATA: %s\n", (char *)data);
}

connection_t con;
void dolisten(void) {
  connection_t cons[1] = { con };
  p2p_listener(&cons, 1, NULL, &callback, NULL);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: p2pvc [server] [port]\n");
  }
  p2p_connect(argv[1], argv[2], &con);

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, NULL);

  return 0;
}

