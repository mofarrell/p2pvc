#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#include <p2plib.h>
#include <display.h>
#include <video.h>

#define GET_HEIGHT(h) (h)
#define GET_WIDTH(w) (((w)/8)*8)

#define COLOR_DEPTH       3
#define BANDWIDTH_BUFLEN  64

static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;
static pthread_mutex_t buffer_lock;
static CvCapture* cv_cap;
static int disp_bandwidth = 0;
static int width;
static int height;
static int depth = COLOR_DEPTH;

static void callback(connection_t *con, void *data, size_t length) {
  pthread_mutex_lock(&buffer_lock);
  unsigned long index = ntohl(((unsigned long*)data)[0]);
  int y = ((length - (sizeof(unsigned long))) * index) / (width * depth);
  draw_line(&(((char*)data)[(sizeof(unsigned long))]), length - (sizeof(unsigned long)), y, depth);
  if (disp_bandwidth) {
    char bandstr[BANDWIDTH_BUFLEN];
    memset(bandstr, 0, BANDWIDTH_BUFLEN);
    sprintf(bandstr, " Bandwidth : %f MB/s", 1000 * p2p_bandwidth(length));
    write_bandwidth(bandstr, strlen(bandstr), width, height);
  }
  pthread_mutex_unlock(&buffer_lock);
}

static void new_callback(connection_t *con, void *data, size_t datalen) {
  pthread_mutex_lock(&conslock);
  conslen++;
  cons = realloc(cons, conslen * sizeof(connection_t));
  memcpy(&(cons[conslen-1]), con, sizeof(connection_t));
  pthread_mutex_unlock(&conslock);
}

void video_shutdown(int signal) {
  cvReleaseCapture( &cv_cap );
  end_screen();
}

static void *dolisten(void *args) {
  int socket;
  int port = atoi((char *)args);
  p2p_init(port, &socket);
  p2p_listener((connection_t **)&cons, &conslen, &conslock, &callback, &new_callback, socket, width * height * depth);
  return NULL;
}

int start_video(char *peer, char *port, vid_options_t *vopt) {

  int w = vopt->width;
  int h = vopt->height;
  disp_bandwidth = vopt->disp_bandwidth;
  init_screen();
  pthread_mutex_init(&conslock, NULL);
  pthread_mutex_init(&buffer_lock, NULL);

  width = GET_WIDTH(w);
  height = GET_HEIGHT(h);

  cons = calloc(1, sizeof(connection_t));
  if (p2p_connect(peer, port, &(cons[0]))) {
    fprintf(stderr, "Unable to connect to server.\n");
  } else {
    conslen++;
  }

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, (void *)port);

  IplImage* color_img;
  IplImage* resize_img = cvCreateImage(cvSize(width, height), 8, 3);  

  cv_cap = cvCaptureFromCAM(0);
  char line_buffer[sizeof(unsigned long) + width * depth];
  struct timespec tim, actual_tim;
  tim.tv_sec = 0;
  tim.tv_nsec = 50000000;
  while (1) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      unsigned long line_index;
      for (line_index = 0; line_index < (resize_img->imageSize / (width * depth)); line_index++) {
        memset(line_buffer, 0, sizeof(line_buffer));
        unsigned long send_index = htonl(line_index);
        memcpy(line_buffer, &send_index, sizeof(unsigned long));
        memcpy(&(line_buffer[sizeof(unsigned long)]), resize_img->imageData + (line_index * width * depth), width * depth);
        p2p_broadcast(&cons, &conslen, &conslock, line_buffer, + sizeof(line_buffer));
      }
      nanosleep(&tim, &actual_tim);
    }
  }

  /* Housekeeping */
  cvReleaseCapture( &cv_cap );
  end_screen();
  return 0;
}

#ifdef VIDEOONLY
int main(int argc, char *argv[]) {
  vid_options_t vopt;
  memset(&vopt, 0, sizeof(vid_options_t));
  vopt.width = 100;
  vopt.height = 40;
  vopt.disp_bandwidth = 0;
  return start_video(argv[1], "55556", &vopt);
}
#endif

