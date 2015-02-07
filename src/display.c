#include "display.h"

WINDOW *main_screen;

/* private functions */
void init_colors(void);

void init_screen(void){
  main_screen = initscr();
  keypad(stdscr, TRUE);           // keypad enabled
  (void) nodelay(main_screen, 1); // no blocking
  (void) noecho();                // don't print to screen
  (void) nonl();                  // no new lines
  init_colors();
}

/*
 * crazy shifting is to set up every color 
 */
void init_colors(void) {
  int i;
  start_color();
  if (COLORS == 8) {
    for (i = 0; i < (1 << 8); i ++) {
      init_pair(i, 255, 0); // 0 --> i if you want pure blocks, otherwise ascii
    }
  } else {
    for (i = 0; i < (1 << 8); i ++) {
      init_pair(i, i, 0); // 0 --> i if you want pure blocks, otherwise ascii
    }
  }
  return;
}

void end_screen(void) {
  endwin();
}

/* allow us to directly map to the 216 colors ncurses makes available */
static inline int get_color(int r, int g, int b) {
  return 16+r/48*36+g/48*6+b/48; 
}

const char *ascii_values = " .:-=+oo*#%@";
/* vector drawer
 */
//int n3_draw_image(char *data, int width, int height) {
int draw_image(char *data, int width, int height, int step, int channels) {
  char ascii_image[width*height];
  int y, x;
  unsigned char b, g, r;
  int offset = 0;
  int intensity;
  for (y=0; y<height; y++){
    for (x=0; x<width; x++){
      b = data[step * y + x * channels] + offset;
      g = data[step * y + x * channels + 1] + offset;
      r = data[step * y + x * channels + 2] + offset;
      intensity = (int)(0.2126*r + 0.7152*g + 0.0722*b);
      ascii_image[y*width + x] = ascii_values[intensity / 25];
      int color = get_color(r,g,b);
      mvaddch(y, x, ascii_image[y*width + x]|COLOR_PAIR(color));
    }
  } 

  refresh();
  return 0;
}

