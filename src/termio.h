#ifndef TERMIO_H
#define TERMIO_H

#include <stddef.h>

int termio_init(int cols, int rows, int scale);
void termio_shutdown(void);

void termio_clear(void);
void termio_set_cursor(int row, int col);
void termio_write(const char *str);
void termio_write_char(char c);
void termio_put_char_at(int row, int col, char c);
void termio_printf(const char *fmt, ...);
void termio_present(void);

/* Blocking line editor. Returns length, 0 for empty line, -1 on EOF/quit. */
int termio_readline(char *buf, int maxlen);

/* LINE EDIT mode: edit a line from the scrollback. Returns length, -1 on cancel. */
int termio_lineedit(int line_num, char *buf, int maxlen);

/* Handle events like scrolling (call periodically when not in readline) */
void termio_handle_events(void);

/* Non-blocking key fetch. Returns character code or -1 if none. */
int termio_poll_key(void);

/* Optional window title (no-op in stdio backend). */
void termio_set_title(const char *title);

/* Render graphics buffer to display (SDL: resize + present; stdio: no-op). */
void termio_render_graphics(void);

/* Play a beep: duration_ms in milliseconds, freq_hz in Hz (or 0/1/2 for low/mid/high). */
void termio_beep(int duration_ms, int freq_hz);

/* Play a tone with harmonics: base_freq plus harmonic overtones with specified intensities */
void termio_sound_harmonics(int base_freq, const int *harmonics, const double *intensities,
                            int num_harmonics, int duration_ms);

/* Set text colors: fg/bg are 0=black, 1=white */
void termio_set_colors(int fg, int bg);

/* Display welcome screen with version information (SDL2 only) */
void termio_show_welcome(const char *name, const char *version);

#endif /* TERMIO_H */