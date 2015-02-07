#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>


void n3_end_screen(void);
void n3_init_screen(void);
int n3_draw_image(char *data, int width, int height);
int n3_draw_xy(char c, int x, int y);
int n3_getch(void);
void n3_refresh(void);

#endif /* DISPLAY_H */
