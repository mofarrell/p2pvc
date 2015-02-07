#include <stdio.h>
#include <p2plib.h>

void callback(connection_t *con, void *data, size_t datalen) {
  if ((int)datalen > 0) {
    printf("DATA(%d): %s\n", datalen, (char *)data);
  }
}

int main(void) {
  int socket;
  p2p_init(55555, &socket);
  connection_t con;
  connection_t cons[1] = { con };
  size_t i = 1;
  p2p_listener((connection_t **)&cons, &i, NULL, &callback, &callback, socket);
  return 1;
}
