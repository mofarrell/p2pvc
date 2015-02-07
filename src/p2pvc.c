#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <p2plib.h>
#include <unistd.h>

#include <audio.h>
#include <video.h>

void *spawn_audio_thread(void *arg) {
  start_audio((char **)arg);
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: p2pvc [server]\n");
    exit(1);
  }

  int spawn_video = 0;
  if (argc > 2 && !strncmp("-v", argv[2], 2)) {
    spawn_video = 1;
  }

  if (spawn_video) {
    pthread_t thr;
    pthread_create(&thr, NULL, spawn_audio_thread, (void *)argv);
    start_video(argv);
  } else {
    start_audio(argv);
  }

  return 0;
}

