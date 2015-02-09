#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <p2plib.h>
#include <unistd.h>
#include <signal.h>

#include <audio.h>
#include <video.h>

typedef struct {
  char *ipaddr;
  char *port;
} network_options_t;

void *spawn_audio_thread(void *args) {
  network_options_t *netopts = args;
  start_audio(netopts->ipaddr, netopts->port);
  return NULL;
}

void audio_shutdown(int signal) {
  kill(getpid(), SIGUSR1);
}

void all_shutdown(int signal) {
  video_shutdown(signal);
  audio_shutdown(signal);
  exit(0);
}

void get_dimensions(char dim[], int *width, int *height) {
  int i;
  char *wstr = (char *)dim, *hstr;
  for (i = 0; i < sizeof(dim); i++) {
    if ((dim[i] == 'x' || dim[i] == ':') && (i + 1 < sizeof(dim))) {
      dim[i] = '\0';
      hstr = &(dim[i+1]);
    }
  }

  *width = atoi(wstr);
  *height = atoi(hstr);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: p2pvc [server] [options]\n");
    exit(1);
  }

  char *peer = NULL;
  char *audio_port = "55555";
  char *video_port = "55556";
  int spawn_video = 0;
  int c;
  int width, height;
  while (optind < argc) {
    if ((c = getopt (argc, argv, "vd:A:V:")) != -1) {
      switch (c) {
        case 'v':
          spawn_video = 1;
          break;
        case 'A':
          audio_port = optarg;
          break;
        case 'V':
          video_port = optarg;
          break;
        case 'd':
          get_dimensions(optarg, &width, &height);
          break;
        default:
          break;
      }
    } else {
      peer = argv[optind];
      optind++;
    }
  }


  if (spawn_video) {
    signal(SIGINT, all_shutdown);
    pthread_t thr;
    network_options_t netopts;
    netopts.ipaddr = peer;
    netopts.port = audio_port;
    pthread_create(&thr, NULL, spawn_audio_thread, (void *)&netopts);
    start_video(peer, video_port, width, height);
  } else {
    signal(SIGINT, audio_shutdown);
    start_audio(peer, audio_port);
  }

  return 0;
}

