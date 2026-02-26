#include "video_backend.h"
#include <string.h>
#include <stdlib.h>

VideoMemory video_memory;
static int g_screen_mode = 0; /* 0=text, 1=graphics */

void video_clear(void)
{
    memset(video_memory.pixels, video_memory.background_color, sizeof(video_memory.pixels));
}

void video_set_pixel(int x, int y, int color)
{
    if (x >= 0 && x < video_memory.width && y >= 0 && y < video_memory.height)
    {
        video_memory.pixels[y][x] = (unsigned char)color;
    }
}

int video_get_pixel(int x, int y)
{
    if (x >= 0 && x < video_memory.width && y >= 0 && y < video_memory.height)
    {
        return (int)video_memory.pixels[y][x];
    }
    return -1;
}

void video_set_color(int color)
{
    if (color >= 0 && color < video_memory.num_colors)
    {
        video_memory.current_color = color;
    }
}

void video_set_background(int color)
{
    if (color >= 0 && color < video_memory.num_colors)
    {
        video_memory.background_color = color;
    }
}

void video_draw_line(int x1, int y1, int x2, int y2, int color)
{
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    while (1)
    {
        video_set_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void video_draw_circle(int x, int y, int radius, int color)
{
    int dx = radius, dy = 0, err = 0;
    while (dx >= dy)
    {
        video_set_pixel(x + dx, y + dy, color);
        video_set_pixel(x + dy, y + dx, color);
        video_set_pixel(x - dy, y + dx, color);
        video_set_pixel(x - dx, y + dy, color);
        video_set_pixel(x - dx, y - dy, color);
        video_set_pixel(x - dy, y - dx, color);
        video_set_pixel(x + dy, y - dx, color);
        video_set_pixel(x + dx, y - dy, color);
        dy++;
        err += 1 + 2 * dy;
        if (2 * err + 1 > 2 * dx)
        {
            dx--;
            err += 1 - 2 * dx;
        }
    }
}

void video_paint(int x, int y, int color)
{
    video_set_pixel(x, y, color);
}

void video_set_screen_mode(int mode)
{
    g_screen_mode = mode;

    if (mode == 0)
    {
        /* Text mode: TRS-80 standard 80x24 text mode */
        video_memory.width = 80;
        video_memory.height = 24;
        video_memory.num_colors = 0;
    }
    else if (mode == 1)
    {
        /* Graphics mode: 320x192 resolution */
        video_memory.width = VIDEO_WIDTH;
        video_memory.height = VIDEO_HEIGHT;
        video_memory.num_colors = VIDEO_MAX_COLORS;
        video_memory.current_color = 0;
        video_memory.background_color = 0;
        video_clear();
    }
}

int video_get_screen_mode(void)
{
    return g_screen_mode;
}

int video_graphics_active(void)
{
    return video_memory.width > 0 && video_memory.height > 0;
}
