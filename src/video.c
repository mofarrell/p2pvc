#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include "display.h"

#define VIDEO_WIDTH 100
#define VIDEO_HEIGHT 40

int start_video(char *argv[]) {
  init_screen();

  IplImage* color_img;
  IplImage* resize_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,3);  

  CvCapture* cv_cap = cvCaptureFromCAM(0);
  for(;;) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      draw_image(resize_img->imageData, resize_img->width, resize_img->height, resize_img->widthStep, resize_img->nChannels);
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
