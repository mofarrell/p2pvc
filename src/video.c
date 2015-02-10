#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#include <p2plib.h>
#include <display.h>
#include <video.h>

//#define VIDEO_WIDTH ((COLS / 8) * 8)
//#define VIDEO_HEIGHT (LINES)

#define GET_HEIGHT(h) (h)
#define GET_WIDTH(w) (((w)/8)*8)

static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;
static pthread_mutex_t buffer_lock;
static CvCapture* cv_cap;
static int disp_bandwidth = 0;
static int width;
static int height;
static int depth = 3;

static void callback(connection_t *con, void *data, size_t length) {
  pthread_mutex_lock(&buffer_lock);
  draw_image((char *)data, width, height, width * depth, depth);
  if (disp_bandwidth) {
    char *bandstr = alloca(64);
    memset(bandstr, 0, 64);
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

  if (w > COLS) {
    width = GET_WIDTH(COLS);
  } else {
    width = GET_WIDTH(w);
  }

  if (h > LINES) {
    height = GET_HEIGHT(LINES);
  } else {
    height = GET_HEIGHT(h);
  }

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
  for(;;) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      p2p_broadcast(&cons, &conslen, &conslock, resize_img->imageData, width * depth * height);
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

