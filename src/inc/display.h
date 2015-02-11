#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>

typedef struct {
  int intensity_threshold;
  double saturation;
  char *ascii_values;
  int monochrome;
  char r; char g; char b;
} display_options_t;

void init_screen(display_options_t *dopt);
void end_screen(void);
int draw_image(char *data, int width, int height, int step, int channels);
int draw_braille(char *data, int width, int y, int channels);
int draw_line(char *data, int width, int y, int channels);
int write_bandwidth(char *bandstr, int bandlen, int width, int height);

#endif /* DISPLAY_H */
