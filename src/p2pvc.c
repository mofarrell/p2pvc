#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <p2plib.h>
#include <unistd.h>

#include <audio.h>
#include <video.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: p2pvc [server]\n");
    exit(1);
  }

  start_audio(argv);
  start_video(argv);

  return 0;
}

