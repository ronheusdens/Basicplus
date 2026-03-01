#include "termio.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/* Check if stdout is a TTY for conditional ANSI output */
static int is_tty_mode(void)
{
    static int cached = -1;
    if (cached == -1)
    {
        cached = isatty(STDOUT_FILENO) ? 1 : 0;
    }
    return cached;
}

int termio_init(int cols, int rows, int scale)
{
    (void)cols;
    (void)rows;
    (void)scale;
    return 1;
}

void termio_shutdown(void)
{
}

void termio_clear(void)
{
    if (is_tty_mode())
    {
        printf("\033[2J\033[H");
        fflush(stdout);
    }
}

void termio_write(const char *str)
{
    if (!str)
        return;
    fputs(str, stdout);
    fflush(stdout);
}

void termio_write_char(char c)
{
    fputc(c, stdout);
    fflush(stdout);
}

void termio_put_char_at(int row, int col, char c)
{
    /* Use ANSI cursor save/restore only in TTY mode */
    if (is_tty_mode())
    {
        printf("\033[s\033[%d;%dH%c\033[u", row + 1, col + 1, c);
    }
    else
    {
        /* In non-TTY mode, just output the character directly */
        fputc(c, stdout);
    }
    fflush(stdout);
}

void termio_printf(const char *fmt, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    termio_write(buf);
}

void termio_present(void)
{
    fflush(stdout);
}

int termio_readline(char *buf, int maxlen)
{
    if (!buf || maxlen <= 0)
        return -1;
    if (fgets(buf, maxlen, stdin) == NULL)
        return -1;
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    {
        buf[--len] = '\0';
    }
    for (size_t i = 0; i < len; i++)
    {
        unsigned char ch = (unsigned char)buf[i];
        buf[i] = (char)toupper(ch);
    }
    return (int)len;
}

int termio_poll_key(void)
{
    return -1;
}

void termio_set_title(const char *title)
{
    (void)title;
}

void termio_render_graphics(void)
{
}

void termio_beep(int duration_ms, int freq_hz)
{
    (void)freq_hz;
    printf("\a");
    fflush(stdout);
    if (duration_ms > 0)
    {
        unsigned int usec = (unsigned int)(duration_ms * 1000);
        usleep(usec);
    }
}

void termio_set_cursor(int row, int col)
{
    /* Set cursor position for TTY mode only */
    if (is_tty_mode())
    {
        printf("\033[%d;%dH", row + 1, col + 1);
        fflush(stdout);
    }
}

void termio_handle_events(void)
{
    /* No-op for stdio backend */
}

int termio_lineedit(int line_num, char *buf, int maxlen)
{
    (void)line_num;
    (void)buf;
    (void)maxlen;
    /* Line editing not supported in stdio backend */
    return -1;
}

void termio_sound_harmonics(int base_freq, const int *harmonics, const double *intensities,
                            int num_harmonics, int duration_ms)
{
    (void)base_freq;
    (void)harmonics;
    (void)intensities;
    (void)num_harmonics;
    /* Play a beep as fallback */
    termio_beep(duration_ms, 0);
}

void termio_set_colors(int fg, int bg)
{
    (void)fg;
    (void)bg;
    /* Set text colors for TTY mode only */
    if (is_tty_mode())
    {
        if (fg == 1 && bg == 0)
        {
            printf("\033[37m\033[40m"); /* white on black */
        }
        else if (fg == 0 && bg == 1)
        {
            printf("\033[30m\033[47m"); /* black on white */
        }
        fflush(stdout);
    }
}

/* Stub: welcome screen not available in stdio backend */
void termio_show_welcome(const char *name, const char *version)
{
    (void)name;
    (void)version;
    /* No-op in stdio mode */
}

void termio_set_write_color(int color_idx)
{
    (void)color_idx; /* No-op in stdio mode */
}

void termio_write_highlighted(const char *line)
{
    if (line)
        printf("%s\n", line);
}