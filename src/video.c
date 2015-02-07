#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#define VIDEO_WIDTH 200
#define VIDEO_HEIGHT 250

#ifdef VIDEOONLY

int main(void) {
  IplImage* color_img;
  IplImage* resize_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,3)    ;   
  IplImage* gray_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,1)    ;   
  CvCapture* cv_cap = cvCaptureFromCAM(0);
  /* Create window */
  cvNamedWindow("Video",0); 
  for(;;) {
    /* Get each frame */
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0) {
      cvResize(color_img, resize_img, CV_INTER_AREA);
      cvCvtColor(resize_img, gray_img, CV_BGR2GRAY);
      /* Display it yo */
      cvShowImage("Video", gray_img);
    }
  }
  /* Housekeeping */
  cvReleaseCapture( &cv_cap );
  cvDestroyWindow("Video");
  return 0;
}
#endif
