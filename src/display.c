#include "display.h"

WINDOW *main_screen;

/* private functions */
void n3_init_colors(void);

void n3_init_screen(void){
  main_screen = initscr();
  keypad(stdscr, TRUE);           // keypad enabled
  (void) nodelay(main_screen, 1); // no blocking
  (void) nonl();                  // no new lines
  (void) noecho();                // don't print to screen
  n3_init_colors();
  refresh();
}

/*
 * crazy shifting is to set up every color 
 */
void n3_init_colors(void) {
  start_color();
  for (int i = 0; i < (1 << 8); i ++) {
    int r = i >> 5;
    int g = (i >> 2) & 0b111;
    int b = i & 0b111;
    init_color(i, r, g, b);
    init_pair(i, i, 0); // 0 --> i if you want pure blocks, otherwise ascii
  }
  return;
}

void n3_end_screen(void) {
  endwin();
}

/* allow us to directly map to the 216 colors ncurses makes available */
static inline int get_color(int r, int g, int b) {
  return 16+r/48*36+g/48*6+b/48; 
}

const char *sizes = "=+*%%@#";

/* vector drawer
 */
int n3_draw_image(char *data, int width, int height) {
  for (int y=0; y<height; y++){
    for (int x=0; x<width; x++){
      int intensity = (data[((height-y-1)*width+x)*4] +
          data[((height-y-1)*width+x)*4+1] + 
          data[((height-y-1)*width+x)*4+2])/127;
      int color = get_color(
          data[((height-y-1)*width+x)*4],
          data[((height-y-1)*width+x)*4+1],
          data[((height-y-1)*width+x)*4+2]);
      //mvaddch(y, x, sizes[intensity]|COLOR_PAIR(color));

    }
  }
  return 0;
}

/* 
 * draws char to (x,y)
 * returns -1 on error
 */
int n3_draw_xy(char c, int x, int y){
  if (x > COLS || x < 0 || y > LINES || y < 0) {
    return -1;
  } else {
    mvaddch(x, y, c);
    return 0;
  }
}
