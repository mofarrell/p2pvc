/* initializes a UDP server, returns 0 on success or a corresponding errno
 * on failure. Populates a given sockaddr_in */
int init_server(struct sockaddr_in *me) {
  int sockfd;

  /* create socket */
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    fprintf(stderr, "error in opening socket: %s\n", strerror(errno));
    return (errno);
  }

  /* XXX: Make sure these fields are OK, possibly switch to variables.*/
  me.sin_family = AF_INET;
  me.sin_port = htons(PORT);
  me.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    fprintf(stderr, "error in binding socket: %s\n", strerror(errno));
    return (errno);
  }

  /* success */
  return (0);
}

/* sample code which can hopefully write and read to another peer */
int main(int argc, char **argv) {

}
