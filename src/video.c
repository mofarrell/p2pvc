#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#include <p2plib.h>
#include <display.h>

#define VIDEO_WIDTH 100
#define VIDEO_HEIGHT 40

static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;
static pthread_mutex_t buffer_lock;

static void callback(connection_t *con, void *data, size_t length) {
  pthread_mutex_lock(&buffer_lock);
  draw_image((char *)data, VIDEO_WIDTH, VIDEO_HEIGHT, 3 * VIDEO_WIDTH, 3);
  pthread_mutex_unlock(&buffer_lock);
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
  int port = atoi((char *)args);
  p2p_init(port, &socket);
  p2p_listener((connection_t **)&cons, &conslen, &conslock, &callback, &new_callback, socket);
  return NULL;
}

int start_video(char *peer, char *port, int width, int height) {
  init_screen();
  pthread_mutex_init(&conslock, NULL);
  pthread_mutex_init(&buffer_lock, NULL);

  cons = calloc(1, sizeof(connection_t));
  if (p2p_connect(peer, port, &(cons[0]))) {
    fprintf(stderr, "Unable to connect to server.\n");
  } else {
    conslen++;
  }

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, (void *)port);

  IplImage* color_img;
  IplImage* resize_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,3);  

  CvCapture* cv_cap = cvCaptureFromCAM(0);
  for(;;) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      p2p_broadcast(&cons, &conslen, &conslock, resize_img->imageData, VIDEO_WIDTH * 3 * VIDEO_HEIGHT);
      //draw_image(resize_img->imageData, resize_img->width, resize_img->height, resize_img->widthStep, resize_img->nChannels);
    }
  }

  /* Housekeeping */
  cvReleaseCapture( &cv_cap );
  cvDestroyWindow("Video");
  end_screen();
  return 0;
}


#ifdef VIDEOONLY
int main(int argc, char *argv[]) {
  return start_video(argv);;
}
#endif
