/***
 * p2pvc
 ***/

#include <p2plib.h>

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <locale.h>

#include <pulse/pulseaudio.h>

static pa_context *context = NULL;
static pa_stream *in_stream = NULL;
static pa_stream *out_stream = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static pa_mainloop *mainloop = NULL;

static char *in_stream_name = "InputStream", *client_name = "AudioChatIn", *in_device = NULL;
static char *out_stream_name = "OutputStream", *out_device = NULL;

int poll = 0;

static int verbose = 1;

#define BUFFER_SIZE (1024*10)
static size_t read_ptr = 0;
static size_t write_ptr = 0;
static uint8_t buffer[BUFFER_SIZE];


static pa_volume_t volume = PA_VOLUME_NORM;

#define CHANNELS 2
static pa_sample_spec sample_spec = { 
  .format = PA_SAMPLE_U8, 
  .rate = 44100,
  .channels = CHANNELS
};

#define LATENCY 4096
#define PROCESS_TIME 1024

static pa_buffer_attr buffer_attr = {
  .maxlength = (uint32_t)-1,
  .tlength = (uint32_t)LATENCY,
  .prebuf = (uint32_t)-1,
  .minreq = (uint32_t)PROCESS_TIME,
  .fragsize = (uint32_t)LATENCY
};


static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;
static pthread_mutex_t buffer_lock;


/* A shortcut for terminating the application */
static void quit(int ret) {
  assert(mainloop_api);
  mainloop_api->quit(mainloop_api, ret);
}

/* This is called whenever new data may be written to the stream */
static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {
  assert(s && length);

  pthread_mutex_lock(&buffer_lock);
  if (verbose) {
    fprintf(stderr, "write length: %lu\n", length);

    fprintf(stderr, "read: %lu, write: %lu, length: %lu\n", read_ptr, write_ptr, length); 
  }
  if (read_ptr < write_ptr && write_ptr + length >= BUFFER_SIZE) {
    length = BUFFER_SIZE - write_ptr;
  } else if (write_ptr <= read_ptr && write_ptr + length >= read_ptr) {
    length = read_ptr - write_ptr;
  }
  if (verbose) {
    fprintf(stderr, "read: %lu, write: %lu, length: %lu\n", read_ptr, write_ptr, length); 
  }

  assert(write_ptr + length <= BUFFER_SIZE);
  if (pa_stream_write(out_stream, (uint8_t*) buffer + write_ptr, length, NULL, 0, PA_SEEK_RELATIVE) < 0) {
    fprintf(stderr, ("pa_stream_write() failed: %s\n"), pa_strerror(pa_context_errno(context)));
    quit(1);
    return;
  }
  write_ptr = (write_ptr + length) % BUFFER_SIZE;
  pthread_mutex_unlock(&buffer_lock);
}

/* This is called whenever new data may be written to the stream */
static void stream_read_callback(pa_stream *s, size_t length, void *userdata) {
  const void *data;
  assert(s && length > 0);

  if (pa_stream_peek(s, &data, &length) < 0) {
    fprintf(stderr, ("Could not peek stream.\n"));
  }
  assert(length > 0);

  if (verbose) {
    fprintf(stderr, "reading: %lu\n", length);
  }
  
  p2p_broadcast(&cons, &conslen, &conslock, data, length); 

  if (pa_stream_drop(s) < 0) {
    fprintf(stderr, ("Could not drop fragment.\n"));
  }
/*
  size_t length1 = MIN(length, BUFFER_SIZE - read_ptr);
  size_t length2 = length - length1;

  if (write_ptr > read_ptr && write_ptr < read_ptr + length1) {
    write_ptr = read_ptr + length1;
  }
  if (length2 != 0 && write_ptr < length2) {
    write_ptr = length2;
  }
  assert(length1 + length2 == length);

  if (verbose) {
    fprintf(stderr, "length1: %lu, length2: %lu\n", length1, length2);
  }

  assert(read_ptr + length1 <= BUFFER_SIZE);
  memcpy(&buffer[read_ptr], data, length1);
  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }
  read_ptr = (read_ptr + length1) % BUFFER_SIZE;
  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }

  if (length2 != 0) {
    assert(length2 <= BUFFER_SIZE);
    memcpy(buffer, (uint8_t *)data + length1, length2);
    read_ptr = length2;
  }

  if (pa_stream_drop(s) < 0) {
    fprintf(stderr, ("Could not drop fragment.\n"));
  }

  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }
  */
}

/* This routine is called whenever the stream state changes */
static void stream_state_callback(pa_stream *s, void *userdata) {
  assert(s);

  switch (pa_stream_get_state(s)) {
    case PA_STREAM_CREATING:
    case PA_STREAM_TERMINATED:
      break;

    case PA_STREAM_READY:
      if (verbose) {
        const pa_buffer_attr *a;
        char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

        if (s == out_stream) {
          poll = 1;
        }

        fprintf(stderr, ("Stream successfully created.\n"));

        if (!(a = pa_stream_get_buffer_attr(s))) {
          fprintf(stderr, ("pa_stream_get_buffer_attr() failed: %s\n"), pa_strerror(pa_context_errno(pa_stream_get_context(s))));
        } else {
          fprintf(stderr, ("Buffer metrics: maxlength=%u, fragsize=%u\n"), a->maxlength, a->fragsize);
        }

        fprintf(stderr, ("Using sample spec '%s', channel map '%s'.\n"),
            pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(s)),
            pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(s)));

        fprintf(stderr, ("Connected to device %s (%u, %ssuspended).\n"),
            pa_stream_get_device_name(s),
            pa_stream_get_device_index(s),
            pa_stream_is_suspended(s) ? "" : "not ");
      }
      break;

    case PA_STREAM_FAILED:
    default:
      fprintf(stderr, ("Stream errror: %s\n"), pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      quit(1);
  }
}

/* This is called whenever the context status changes */
static void context_state_callback(pa_context *c, void *userdata) {
  assert(c);

  switch (pa_context_get_state(c)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
      break;

    case PA_CONTEXT_READY: {
      assert(c && !in_stream && !out_stream);


      if (verbose) {
        fprintf(stderr, ("Connection established.\n"));
      }

      assert(pa_sample_spec_valid(&sample_spec));


      out_stream = pa_stream_new(c, out_stream_name, &sample_spec, NULL);
      if (!out_stream) {
        fprintf(stderr, "No out stream %s\n", pa_strerror(pa_context_errno(context)));
      }

      pa_stream_set_state_callback(out_stream, stream_state_callback, NULL);
      pa_stream_set_write_callback(out_stream, stream_write_callback, NULL);
      pa_cvolume cvolume;
      pa_stream_connect_playback(out_stream, out_device, &buffer_attr, PA_STREAM_ADJUST_LATENCY, pa_cvolume_set(&cvolume, CHANNELS, volume), NULL);


      in_stream = pa_stream_new(c, in_stream_name, &sample_spec, NULL);
      if (!in_stream) {
        fprintf(stderr, "No stream %s\n", pa_strerror(pa_context_errno(context)));
      }
      assert(in_stream);

      pa_stream_set_state_callback(in_stream, stream_state_callback, NULL);
      pa_stream_set_read_callback(in_stream, stream_read_callback, NULL);
      pa_stream_connect_record(in_stream, in_device, &buffer_attr, PA_STREAM_ADJUST_LATENCY);
      break;
    }

    case PA_CONTEXT_TERMINATED:
                           quit(0);
                           break;

    case PA_CONTEXT_FAILED:
    default:
                           fprintf(stderr, ("Connection failure: %s\n"), pa_strerror(pa_context_errno(c)));
                           quit(1);
  }
}

/* UNIX signal to quit recieved */
static void exit_signal_callback(pa_mainloop_api*m, pa_signal_event *e, int sig, void *userdata) {
  if (verbose)
    fprintf(stderr, ("Got SIGINT, exiting.\n"));
  quit(0);
}

static void callback(connection_t *con, void *data, size_t length) {
  pthread_mutex_lock(&buffer_lock);

  size_t length1 = MIN(length, BUFFER_SIZE - read_ptr);
  size_t length2 = length - length1;

  if (write_ptr > read_ptr && write_ptr < read_ptr + length1) {
    write_ptr = read_ptr + length1;
  }
  if (length2 != 0 && write_ptr < length2) {
    write_ptr = length2;
  }
  assert(length1 + length2 == length);

  if (verbose) {
    fprintf(stderr, "length1: %lu, length2: %lu\n", length1, length2);
  }

  assert(read_ptr + length1 <= BUFFER_SIZE);
  memcpy(&buffer[read_ptr], data, length1);
  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }
  read_ptr = (read_ptr + length1) % BUFFER_SIZE;
  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }

  if (length2 != 0) {
    assert(length2 <= BUFFER_SIZE);
    memcpy(buffer, (uint8_t *)data + length1, length2);
    read_ptr = length2;
  }

  if (verbose) {
    fprintf(stderr, "read: %lu\n", read_ptr);
  }
  pthread_mutex_unlock(&buffer_lock);

  kill(getpid(), SIGALRM);
}

static void new_callback(connection_t *con, void *data, size_t datalen) {
  pthread_mutex_lock(&conslock);
  conslen++;
  cons = realloc(cons, conslen * sizeof(connection_t));
  memcpy(&(cons[conslen-1]), con, sizeof(connection_t));
  pthread_mutex_unlock(&conslock);
}

static void *dolisten(void *args) {
  int socket;
  p2p_init(55555, &socket);
  p2p_listener((connection_t **)&cons, &conslen, &conslock, &callback, &new_callback, socket);
  return NULL;
}

static void audio_poll(pa_mainloop_api*m, pa_signal_event *e, int sig, void *userdata) {
  if (out_stream) {
    size_t length = pa_stream_writable_size(out_stream);
    if (length > 0) {
      stream_write_callback(out_stream, length, NULL);
    }
  }

  return;
}

/* Starts audio listenning and emission. */
int start_audio(char *argv[]) {
  pthread_mutex_init(&conslock, NULL);
  pthread_mutex_init(&buffer_lock, NULL);

  cons = calloc(1, sizeof(connection_t));
  if (p2p_connect(argv[1], "55555", &(cons[0]))) {
    fprintf(stderr, "Unable to connect to server.\n");
  } else {
    conslen++;
  }

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, NULL);

  /* Check that our buffer is big enough. */
  assert(LATENCY < BUFFER_SIZE && PROCESS_TIME < BUFFER_SIZE);

  int ret = 1, r;
  char *server = NULL;

  /* Set up a new main loop */
  if (!(mainloop = pa_mainloop_new())) {
    fprintf(stderr, ("pa_mainloop_new() failed.\n"));
    goto quit;
  }

  mainloop_api = pa_mainloop_get_api(mainloop);

  r = pa_signal_init(mainloop_api);
  assert(r == 0);
  pa_signal_new(SIGINT, &exit_signal_callback, NULL);
  pa_signal_new(SIGALRM, &audio_poll, NULL);
#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif

  /* Create a new connection context. */
  if (!(context = pa_context_new(mainloop_api, client_name))) {
    fprintf(stderr, ("pa_context_new() failed.\n"));
    goto quit;
  }

  pa_context_set_state_callback(context, context_state_callback, NULL);

  /* Connect the context */
  if (pa_context_connect(context, server, 0, NULL) < 0) {
    fprintf(stderr, ("pa_context_connect() failed: %s"), pa_strerror(pa_context_errno(context)));
    goto quit;
  }

  /* Run the main loop */
  if (pa_mainloop_run(mainloop, &ret) < 0) {
    fprintf(stderr, ("pa_mainloop_run() failed.\n"));
    goto quit;
  }

quit:
  if (in_stream)
    pa_stream_unref(in_stream);

  if (context)
    pa_context_unref(context);

  if (mainloop) {
    pa_signal_done();
    pa_mainloop_free(mainloop);
  }

  pthread_mutex_destroy(&conslock);
  pthread_mutex_destroy(&buffer_lock);
  return ret;
}

#ifdef AUDIOONLY
int main(int argc, char *argv[]) {
  return start_audio(argc, argv);
}
#endif

