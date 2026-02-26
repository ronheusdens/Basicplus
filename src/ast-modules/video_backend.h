#ifndef VIDEO_BACKEND_H
#define VIDEO_BACKEND_H

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 192
#define VIDEO_MAX_COLORS 16

typedef struct VideoMemory
{
    unsigned char pixels[VIDEO_HEIGHT][VIDEO_WIDTH];
    int width;
    int height;
    int num_colors;
    int current_color;
    int background_color;
    int palette[VIDEO_MAX_COLORS];
} VideoMemory;

extern VideoMemory video_memory;

void video_set_color(int color);
void video_set_background(int color);
void video_set_pixel(int x, int y, int color);
int video_get_pixel(int x, int y);
void video_draw_line(int x1, int y1, int x2, int y2, int color);
void video_draw_circle(int x, int y, int radius, int color);
void video_paint(int x, int y, int color);
void video_set_screen_mode(int mode);
int video_get_screen_mode(void);
void video_clear(void);

/* Returns 1 if graphics mode is active (width/height set) */
int video_graphics_active(void);

#endif /* VIDEO_BACKEND_H */
