#ifndef VIDEO_H
#define VIDEO_H

typedef struct {
  int width;
  int height;
  int depth;
  int disp_bandwidth;
  int render_type;
  unsigned long refresh_rate;
  int intensity_threshold;
  double saturation;
  int monochrome;
  char r; char g; char b;
  char *ascii_values;
  int edge_filter; int edge_lower; int edge_upper;
} vid_options_t;

void video_shutdown(int signal);

int start_video(char *peer, char *port, vid_options_t *vopt);

#endif /* VIDEO_H */
