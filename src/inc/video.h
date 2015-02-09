#ifndef VIDEO_H
#define VIDEO_H

void video_shutdown(int signal);

int start_video(char *peer, char *port, int width, int height);

#endif /* VIDEO_H */
