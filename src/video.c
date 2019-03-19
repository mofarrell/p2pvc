#include <opencv2/videoio/videoio_c.h>
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
static CvCapture* cv_cap;
static int disp_bandwidth = 0;
static int width;
static int height;
static int depth = COLOR_DEPTH;
static int render_type; /* 0 default, 1 braille */

static void callback(connection_t *con, void *data, size_t length) {
  unsigned long index = ntohl(((unsigned long*)data)[0]);
  int y = index;

  if (render_type == 1) {
    draw_braille(&(((char*)data)[(sizeof(unsigned long))]), length - (sizeof(unsigned long)), y, depth);
  } else {
    draw_line(&(((char*)data)[(sizeof(unsigned long))]), length - (sizeof(unsigned long)), y, depth);
  }

  if (disp_bandwidth) {
    char bandstr[BANDWIDTH_BUFLEN];
    memset(bandstr, 0, BANDWIDTH_BUFLEN);
    sprintf(bandstr, " Bandwidth : %f MB/s", 1000 * p2p_bandwidth());
    write_bandwidth(bandstr, strlen(bandstr), width, height);
  }
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
  width = GET_WIDTH(vopt->width);
  height = GET_HEIGHT(vopt->height);
  render_type = vopt->render_type;
  disp_bandwidth = vopt->disp_bandwidth;

  display_options_t dopt;
  memset(&dopt, 0, sizeof(display_options_t));

  dopt.intensity_threshold = vopt->intensity_threshold;
  dopt.saturation = vopt->saturation;
  dopt.monochrome = vopt->monochrome;
  dopt.r = vopt->r;
  dopt.g = vopt->g;
  dopt.b = vopt->b;
  dopt.ascii_values = vopt->ascii_values;

  init_screen(&dopt);
  curs_set(0);
  pthread_mutex_init(&conslock, NULL);

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
  IplImage* gray_img = cvCreateImage(cvSize(width, height), 8, 3);  
  IplImage* edge = cvCreateImage(cvGetSize(resize_img), IPL_DEPTH_8U, 1);

  cv_cap = cvCaptureFromCAM(0);
  char line_buffer[sizeof(unsigned long) + width * depth];
  struct timespec tim, actual_tim;
  tim.tv_sec = 0;
  tim.tv_nsec = (1000000000 - 1) / vopt->refresh_rate;
  int kernel = 7;
  while (1) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img && resize_img) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      if (vopt->edge_filter) {
        cvCvtColor(resize_img, edge, CV_BGR2GRAY);
        cvCanny(edge, edge, vopt->edge_lower * kernel * kernel, vopt->edge_upper * kernel * kernel, kernel);
        cvNot(edge, edge);
        cvCvtColor(edge, gray_img, CV_GRAY2BGR);
        cvAnd(gray_img, resize_img, resize_img, NULL);
      }
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
  cvReleaseCapture(&cv_cap);
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

