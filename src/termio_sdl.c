/*
 * Minimal SDL2 Terminal I/O for Basic++ REPL
 * Text-only, no graphics backend
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "termio.h"

#define TERM_COLS 132
#define TERM_ROWS 32
#define FONT_SIZE 16

static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static TTF_Font *g_font = NULL;
static int g_sdl_enabled = 0;
static int g_char_width = 10;
static int g_char_height = 20;
static float g_dpi_scale = 1.0f;

/* Text buffer */
static char g_screen[TERM_ROWS][TERM_COLS + 1];
static Uint8 g_attr[TERM_ROWS][TERM_COLS]; /* per-cell color index */
static Uint8 g_write_color = 0;            /* current write color  */
static int g_cursor_row = 0;
static int g_cursor_col = 0;
/* Monokai Pro (Filter Ristretto) palette
   bg: #2c2525  fg: #fff1f3 */
static SDL_Color g_fg_color = {255, 241, 243, 255}; /* #fff1f3 warm white  */
static SDL_Color g_bg_color = {44, 37, 37, 255};    /* #2c2525 dark brown */

/* Syntax-highlight palette — same indices as COL_* in termio.h */
static const SDL_Color g_palette[] = {
    {255, 241, 243, 255}, /* 0 COL_NORMAL   #fff1f3 warm white  */
    {253, 104, 131, 255}, /* 1 COL_KEYWORD  #fd6883 pink/red    */
    {249, 204, 108, 255}, /* 2 COL_STRING   #f9cc6c yellow      */
    {168, 169, 235, 255}, /* 3 COL_NUMBER   #a8a9eb lavender    */
    {114, 105, 106, 255}, /* 4 COL_COMMENT  #72696a gray        */
    {173, 218, 120, 255}, /* 5 COL_PROCNAME #adda78 green       */
    {148, 138, 139, 255}, /* 6 COL_OPERATOR #948a8b muted grey  */
};

/* ------------------------------------------------------------------ */
/* Scrollback buffer                                                     */
/* ------------------------------------------------------------------ */
#define SCROLLBACK_MAX 2000
#define SCROLLBAR_W_PX 20 /* native pixels for the scrollbar track */

static char g_scrollback[SCROLLBACK_MAX][TERM_COLS + 1];
static Uint8 g_scrollback_attr[SCROLLBACK_MAX][TERM_COLS];
static int g_scrollback_start = 0; /* circular buffer head index          */
static int g_scrollback_count = 0; /* number of valid entries             */
static int g_scroll_offset = 0;    /* 0 = live view (bottom)              */
static int g_scroll_line = -1;     /* highlighted row (-1 = none)         */

/* Push g_screen row 0 into the scrollback ring before evicting it. */
static void scrollback_push(void)
{
    int idx;
    if (g_scrollback_count < SCROLLBACK_MAX)
    {
        idx = (g_scrollback_start + g_scrollback_count) % SCROLLBACK_MAX;
        g_scrollback_count++;
    }
    else
    {
        /* Oldest entry overwritten — advance start */
        idx = g_scrollback_start;
        g_scrollback_start = (g_scrollback_start + 1) % SCROLLBACK_MAX;
    }
    memcpy(g_scrollback[idx], g_screen[0], TERM_COLS);
    g_scrollback[idx][TERM_COLS] = '\0';
    memcpy(g_scrollback_attr[idx], g_attr[0], TERM_COLS);

    /* If user is viewing history, keep the view stable */
    if (g_scroll_offset > 0)
    {
        g_scroll_offset++;
        if (g_scroll_offset > g_scrollback_count)
            g_scroll_offset = g_scrollback_count;
    }
}

/* Adjust scroll offset; clamp to valid range. */
static void scroll_by(int delta)
{
    g_scroll_offset += delta;
    if (g_scroll_offset < 0)
        g_scroll_offset = 0;
    if (g_scroll_offset > g_scrollback_count)
        g_scroll_offset = g_scrollback_count;
}

/* Accumulated fractional scroll from trackpad smooth events */
static float g_scroll_accum = 0.0f;

/* Returns 1 if the event was a scroll action and was consumed. */
static int handle_scroll_event(const SDL_Event *e)
{
    if (e->type == SDL_MOUSEWHEEL)
    {
        /* Use preciseY (SDL 2.0.18+) for smooth trackpad scrolling;
           each trackpad tick sends a small float rather than integer 1. */
        float dy = e->wheel.preciseY;
        if (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
            dy = -dy;
        g_scroll_accum += dy * 3.0f;
        int steps = (int)g_scroll_accum;
        if (steps != 0)
        {
            scroll_by(steps);
            g_scroll_accum -= (float)steps;
        }
        return 1;
    }
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT)
    {
        /* Click inside the scrollbar track -> jump to that position */
        if (!g_renderer)
            return 0;
        int out_w, out_h;
        SDL_GetRendererOutputSize(g_renderer, &out_w, &out_h);
        int sb_w = (int)(SCROLLBAR_W_PX * g_dpi_scale);
        /* SDL mouse coords are logical pixels; convert to physical */
        int click_x = (int)(e->button.x * g_dpi_scale);
        int click_y = (int)(e->button.y * g_dpi_scale);
        if (click_x >= out_w - sb_w && out_h > 0)
        {
            float frac = 1.0f - (float)click_y / (float)out_h;
            int total_lines = g_scrollback_count + TERM_ROWS;
            float max_offset = (float)(total_lines - TERM_ROWS);
            g_scroll_offset = (int)(frac * max_offset);
            if (g_scroll_offset < 0)
                g_scroll_offset = 0;
            if (g_scroll_offset > g_scrollback_count)
                g_scroll_offset = g_scrollback_count;
            return 1;
        }
    }
    if (e->type == SDL_KEYDOWN)
    {
        switch (e->key.keysym.sym)
        {
        case SDLK_PAGEUP:
            scroll_by(TERM_ROWS);
            g_scroll_line = 0;
            return 1;
        case SDLK_PAGEDOWN:
            scroll_by(-TERM_ROWS);
            if (g_scroll_offset == 0)
                g_scroll_line = -1;
            else
                g_scroll_line = TERM_ROWS - 1;
            return 1;
        case SDLK_HOME:
            scroll_by(g_scrollback_count);
            g_scroll_line = 0;
            return 1;
        case SDLK_END:
            g_scroll_offset = 0;
            g_scroll_line = -1;
            return 1;
        case SDLK_ESCAPE:
            if (g_scroll_offset > 0)
            {
                g_scroll_offset = 0;
                g_scroll_line = -1;
                return 1;
            }
            break;
        case SDLK_UP:
            if (g_scroll_offset == 0 && g_scroll_line < 0)
            {
                /* Enter scroll mode: expose one line of history at bottom */
                scroll_by(1);
                g_scroll_line = TERM_ROWS - 1;
                return 1;
            }
            if (g_scroll_offset > 0)
            {
                if (g_scroll_line > 0)
                    g_scroll_line--;
                else
                {
                    scroll_by(1); /* scroll view up, keep line at top */
                    g_scroll_line = 0;
                }
                return 1;
            }
            break;
        case SDLK_DOWN:
            if (g_scroll_offset > 0)
            {
                if (g_scroll_line < TERM_ROWS - 1)
                    g_scroll_line++;
                else
                {
                    scroll_by(-1); /* scroll view down */
                    if (g_scroll_offset == 0)
                        g_scroll_line = -1; /* back to live */
                    else
                        g_scroll_line = TERM_ROWS - 1;
                }
                return 1;
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

int termio_init(int cols, int rows, int scale)
{
    (void)cols;
    (void)rows;
    (void)scale;
    if (g_sdl_enabled)
        return 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    /* Load font first so we can measure character dimensions */
    const char *font_paths[] = {
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/SFNSMono.ttf",
        "/System/Library/Fonts/Supplemental/Andale Mono.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        NULL};

    for (int i = 0; font_paths[i]; i++)
    {
        g_font = TTF_OpenFontIndex(font_paths[i], FONT_SIZE, 0);
        if (g_font)
            break;
    }

    if (!g_font)
    {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    /* Measure actual char dimensions from font metrics */
    {
        int advance = 0;
        TTF_GlyphMetrics(g_font, 'M', NULL, NULL, NULL, NULL, &advance);
        if (advance > 0)
            g_char_width = advance;
        else
            g_char_width = FONT_SIZE * 6 / 10;
        g_char_height = TTF_FontLineSkip(g_font);
    }

    int width = g_char_width * TERM_COLS;
    int height = g_char_height * TERM_ROWS;

    g_window = SDL_CreateWindow(
        "Basic++",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!g_window)
    {
        TTF_CloseFont(g_font);
        g_font = NULL;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer)
    {
        SDL_DestroyWindow(g_window);
        TTF_CloseFont(g_font);
        g_font = NULL;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    /* Detect HiDPI scale (Retina = 2.0) and reload font at native pixels */
    {
        int draw_w = width, win_w = width;
        SDL_GL_GetDrawableSize(g_window, &draw_w, NULL);
        SDL_GetWindowSize(g_window, &win_w, NULL);
        g_dpi_scale = (win_w > 0) ? (float)draw_w / (float)win_w : 1.0f;
    }
    if (g_dpi_scale > 1.0f)
    {
        /* Reload font at scaled size for crisp native-pixel rendering */
        const char *font_paths2[] = {
            "/System/Library/Fonts/Menlo.ttc",
            "/System/Library/Fonts/SFNSMono.ttf",
            "/System/Library/Fonts/Supplemental/Andale Mono.ttf",
            "/System/Library/Fonts/Supplemental/Courier New.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
            NULL};
        int scaled_size = (int)(FONT_SIZE * g_dpi_scale + 0.5f);
        TTF_Font *scaled = NULL;
        for (int i = 0; font_paths2[i]; i++)
        {
            scaled = TTF_OpenFontIndex(font_paths2[i], scaled_size, 0);
            if (scaled)
                break;
        }
        if (scaled)
        {
            TTF_CloseFont(g_font);
            g_font = scaled;
            /* Recompute char dimensions at scaled size */
            int advance = 0;
            TTF_GlyphMetrics(g_font, 'M', NULL, NULL, NULL, NULL, &advance);
            g_char_width = (advance > 0) ? advance : (int)(FONT_SIZE * g_dpi_scale * 6.0f / 10.0f);
            g_char_height = TTF_FontLineSkip(g_font);
        }
    }

    /* Initialize screen buffer */
    for (int i = 0; i < TERM_ROWS; i++)
    {
        memset(g_screen[i], ' ', TERM_COLS);
        memset(g_attr[i], 0, TERM_COLS);
    }

    g_sdl_enabled = 1;
    g_cursor_row = 0;
    g_cursor_col = 0;

    return 0;
}

void termio_shutdown(void)
{
    if (!g_sdl_enabled)
        return;

    if (g_font)
        TTF_CloseFont(g_font);
    if (g_renderer)
        SDL_DestroyRenderer(g_renderer);
    if (g_window)
        SDL_DestroyWindow(g_window);

    TTF_Quit();
    SDL_Quit();
    g_sdl_enabled = 0;
}

void termio_clear(void)
{
    if (!g_sdl_enabled)
        return;

    for (int i = 0; i < TERM_ROWS; i++)
    {
        memset(g_screen[i], ' ', TERM_COLS);
        memset(g_attr[i], 0, TERM_COLS);
    }

    g_cursor_row = 0;
    g_cursor_col = 0;
    g_scroll_offset = 0; /* jump to live view on clear */
    g_scroll_line = -1;
}

void termio_set_cursor(int row, int col)
{
    if (!g_sdl_enabled)
        return;

    if (row < 0)
        row = 0;
    if (row >= TERM_ROWS)
        row = TERM_ROWS - 1;
    if (col < 0)
        col = 0;
    if (col >= TERM_COLS)
        col = TERM_COLS - 1;

    g_cursor_row = row;
    g_cursor_col = col;
}

void termio_put_char_at(int row, int col, char c)
{
    if (!g_sdl_enabled)
        return;

    if (row < 0 || row >= TERM_ROWS || col < 0 || col >= TERM_COLS)
        return;

    g_screen[row][col] = c;
}

void termio_write_char(char c)
{
    if (!g_sdl_enabled)
    {
        putchar(c);
        return;
    }

    if (c == '\n')
    {
        g_cursor_row++;
        g_cursor_col = 0;

        if (g_cursor_row >= TERM_ROWS)
        {
            /* Save top row to scrollback, then scroll up */
            scrollback_push();
            for (int i = 0; i < TERM_ROWS - 1; i++)
            {
                memcpy(g_screen[i], g_screen[i + 1], TERM_COLS);
                memcpy(g_attr[i], g_attr[i + 1], TERM_COLS);
            }
            memset(g_screen[TERM_ROWS - 1], ' ', TERM_COLS);
            memset(g_attr[TERM_ROWS - 1], 0, TERM_COLS);
            g_cursor_row = TERM_ROWS - 1;
        }
    }
    else if (c == '\r')
    {
        g_cursor_col = 0;
    }
    else if (c == '\t')
    {
        /* Advance to next 4-column tab stop */
        int next_stop = ((g_cursor_col / 4) + 1) * 4;
        if (next_stop > TERM_COLS)
            next_stop = TERM_COLS;
        while (g_cursor_col < next_stop)
        {
            g_screen[g_cursor_row][g_cursor_col] = ' ';
            g_attr[g_cursor_row][g_cursor_col] = 0;
            g_cursor_col++;
        }
    }
    else
    {
        if (g_cursor_col >= TERM_COLS)
        {
            g_cursor_row++;
            g_cursor_col = 0;

            if (g_cursor_row >= TERM_ROWS)
            { /* Save top row to scrollback, then scroll up */
                scrollback_push();
                for (int i = 0; i < TERM_ROWS - 1; i++)
                {
                    memcpy(g_screen[i], g_screen[i + 1], TERM_COLS);
                    memcpy(g_attr[i], g_attr[i + 1], TERM_COLS);
                }
                memset(g_screen[TERM_ROWS - 1], ' ', TERM_COLS);
                memset(g_attr[TERM_ROWS - 1], 0, TERM_COLS);
                g_cursor_row = TERM_ROWS - 1;
            }
        }

        g_screen[g_cursor_row][g_cursor_col] = c;
        g_attr[g_cursor_row][g_cursor_col] = g_write_color;
        g_cursor_col++;
    }
}

void termio_write(const char *str)
{
    if (!g_sdl_enabled)
    {
        printf("%s", str);
        return;
    }

    while (*str)
        termio_write_char(*str++);
}

void termio_printf(const char *fmt, ...)
{
    if (!g_sdl_enabled)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        return;
    }

    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    termio_write(buf);
}

void termio_present(void)
{
    if (!g_sdl_enabled || !g_renderer || !g_font)
        return;

    SDL_SetRenderDrawColor(g_renderer, g_bg_color.r, g_bg_color.g, g_bg_color.b, 255);
    SDL_RenderClear(g_renderer);

    /* Focused-line highlight (drawn first, behind text) */
    if (g_scroll_line >= 0 && g_scroll_offset > 0)
    {
        int out_w, out_h_tmp;
        SDL_GetRendererOutputSize(g_renderer, &out_w, &out_h_tmp);
        SDL_Rect hl = {0, g_scroll_line * g_char_height, out_w, g_char_height};
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 255, 241, 243, 28); /* faint warm white */
        SDL_RenderFillRect(g_renderer, &hl);
        /* Left-edge accent bar */
        SDL_Rect bar = {0, g_scroll_line * g_char_height, (int)(3 * g_dpi_scale), g_char_height};
        SDL_SetRenderDrawColor(g_renderer, 253, 104, 131, 180); /* COL_KEYWORD pink */
        SDL_RenderFillRect(g_renderer, &bar);
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
    }

    /* Determine first absolute line to display (scrollback + screen unified) */
    int first_line = g_scrollback_count - g_scroll_offset;

    for (int row = 0; row < TERM_ROWS; row++)
    {
        int abs_line = first_line + row;
        const char *chars;
        const Uint8 *attrs;
        char blank_chars[TERM_COLS + 1];
        Uint8 blank_attrs[TERM_COLS];

        if (abs_line < 0)
        {
            /* Above all history — blank row */
            memset(blank_chars, ' ', TERM_COLS);
            blank_chars[TERM_COLS] = 0;
            memset(blank_attrs, 0, TERM_COLS);
            chars = blank_chars;
            attrs = blank_attrs;
        }
        else if (abs_line < g_scrollback_count)
        {
            /* From scrollback ring  */
            int sb_idx = (g_scrollback_start + abs_line) % SCROLLBACK_MAX;
            chars = g_scrollback[sb_idx];
            attrs = g_scrollback_attr[sb_idx];
        }
        else
        {
            /* From live screen */
            int screen_row = abs_line - g_scrollback_count;
            if (screen_row >= TERM_ROWS)
                continue;
            chars = g_screen[screen_row];
            attrs = g_attr[screen_row];
        }

        for (int col = 0; col < TERM_COLS; col++)
        {
            char c = chars[col];
            if (c == '\0')
                break;
            if (c == ' ')
                continue;

            char text[2] = {c, 0};
            Uint8 ci = attrs[col];
            SDL_Color cell_color = g_palette[ci < 7 ? ci : 0];
            SDL_Surface *surf = TTF_RenderText_Blended(g_font, text, cell_color);
            if (!surf)
                continue;

            SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
            SDL_Rect rect = {
                (int)(col * g_char_width),
                (int)(row * g_char_height),
                g_char_width,
                g_char_height};
            SDL_RenderCopy(g_renderer, tex, NULL, &rect);
            SDL_DestroyTexture(tex);
            SDL_FreeSurface(surf);
        }
    }

    /* Draw scrollbar when there is scrollback content */
    if (g_scrollback_count > 0)
    {
        int out_w, out_h;
        SDL_GetRendererOutputSize(g_renderer, &out_w, &out_h);
        int sb_w = (int)(SCROLLBAR_W_PX * g_dpi_scale);

        /* Track */
        SDL_Rect track = {out_w - sb_w, 0, sb_w, out_h};
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 60, 50, 50, 180);
        SDL_RenderFillRect(g_renderer, &track);

        /* Thumb */
        int total_lines = g_scrollback_count + TERM_ROWS;
        float thumb_frac = (float)TERM_ROWS / (float)total_lines;
        int thumb_h = (int)(out_h * thumb_frac);
        if (thumb_h < 16)
            thumb_h = 16;

        float max_offset = (float)(total_lines - TERM_ROWS);
        float scroll_frac = (max_offset > 0) ? ((float)g_scroll_offset / max_offset) : 0.0f;
        /* offset=0 -> thumb at bottom; offset=max -> thumb at top */
        int thumb_y = (int)((1.0f - scroll_frac) * (float)(out_h - thumb_h));

        SDL_Rect thumb = {out_w - sb_w + 2, thumb_y + 2, sb_w - 4, thumb_h - 4};
        SDL_SetRenderDrawColor(g_renderer, 180, 165, 165, 200);
        SDL_RenderFillRect(g_renderer, &thumb);
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
    }

    SDL_RenderPresent(g_renderer);
}

void termio_set_colors(int fg, int bg)
{
    if (fg == 1 && bg == 0)
    {
        /* WOB: restore Ristretto defaults */
        g_fg_color = (SDL_Color){255, 241, 243, 255}; /* #fff1f3 */
        g_bg_color = (SDL_Color){44, 37, 37, 255};    /* #2c2525 */
    }
    else if (fg == 0 && bg == 1)
    {
        /* BOW: inverted — near-white background, dark text */
        g_fg_color = (SDL_Color){44, 37, 37, 255};    /* #2c2525 */
        g_bg_color = (SDL_Color){255, 241, 243, 255}; /* #fff1f3 */
    }
    termio_present();
}

void termio_handle_events(void)
{
    if (!g_sdl_enabled)
        return;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            exit(0);
        handle_scroll_event(&event);
    }
}

int termio_readline(char *buf, int maxlen)
{
    if (!g_sdl_enabled)
    {
        if (fgets(buf, maxlen, stdin) == NULL)
            return -1;
        int len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[--len] = 0;
        return len;
    }

    int pos = 0;
    int blink_state = 0;
    Uint32 last_blink = SDL_GetTicks();
    int cursor_row = g_cursor_row;
    int cursor_col = g_cursor_col;

    while (1)
    {
        /* Blink the underscore cursor */
        Uint32 now = SDL_GetTicks();
        if (now - last_blink >= 500)
        {
            blink_state = !blink_state;
            last_blink = now;
            g_screen[cursor_row][cursor_col] = blink_state ? '_' : ' ';
        }

        termio_present();

        SDL_Event event;
        if (SDL_WaitEventTimeout(&event, 50))
        {
            if (event.type == SDL_QUIT)
                exit(0);

            /* Scroll wheel / PgUp / PgDn scroll history without disrupting input */
            if (handle_scroll_event(&event))
                continue;

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    /* Clear cursor before accepting */
                    g_screen[cursor_row][cursor_col] = ' ';
                    buf[pos] = 0;
                    termio_write_char('\n');
                    return pos;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && pos > 0)
                {
                    /* Clear cursor, step back */
                    g_screen[cursor_row][cursor_col] = ' ';
                    pos--;
                    g_cursor_col--;
                    if (g_cursor_col < 0)
                    {
                        g_cursor_row--;
                        g_cursor_col = TERM_COLS - 1;
                    }
                    g_screen[g_cursor_row][g_cursor_col] = ' ';
                    cursor_row = g_cursor_row;
                    cursor_col = g_cursor_col;
                    blink_state = 1;
                    last_blink = SDL_GetTicks();
                }
                else if (event.key.keysym.sym == SDLK_c &&
                         (event.key.keysym.mod & KMOD_CTRL))
                {
                    g_screen[cursor_row][cursor_col] = ' ';
                    buf[0] = 0;
                    termio_write_char('\n');
                    return -2; /* Signal Ctrl-C / interrupt */
                }
                else if (event.key.keysym.sym == SDLK_v &&
                         (event.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)))
                {
                    /* Paste clipboard text */
                    char *clip = SDL_GetClipboardText();
                    if (clip)
                    {
                        for (int ci = 0; clip[ci] != '\0'; ci++)
                        {
                            char c = clip[ci];
                            if (c == '\n' || c == '\r')
                                break; /* paste only first line */
                            if (pos < maxlen - 1 && c >= 32 && c < 127)
                            {
                                g_screen[cursor_row][cursor_col] = ' ';
                                buf[pos++] = c;
                                termio_write_char(c);
                                cursor_row = g_cursor_row;
                                cursor_col = g_cursor_col;
                            }
                        }
                        SDL_free(clip);
                        blink_state = 1;
                        last_blink = SDL_GetTicks();
                    }
                }
            }

            if (event.type == SDL_TEXTINPUT)
            {
                char c = event.text.text[0];
                if (pos < maxlen - 1 && c >= 32 && c < 127)
                {
                    /* Clear cursor cell before writing char */
                    g_screen[cursor_row][cursor_col] = ' ';
                    buf[pos++] = c;
                    termio_write_char(c);
                    cursor_row = g_cursor_row;
                    cursor_col = g_cursor_col;
                    blink_state = 1;
                    last_blink = SDL_GetTicks();
                }
            }
        }
    }
}

int termio_lineedit(int line_num, char *buf, int maxlen)
{
    (void)line_num;
    return termio_readline(buf, maxlen);
}

int termio_poll_key(void)
{
    return -1;
}

void termio_set_title(const char *title)
{
    if (!g_sdl_enabled || !g_window)
        return;
    SDL_SetWindowTitle(g_window, title);
}

void termio_beep(int duration_ms, int freq_hz)
{
    (void)duration_ms;
    (void)freq_hz;
}

void termio_sound_harmonics(int base_freq, const int *harmonics, const double *intensities,
                            int num_harmonics, int duration_ms)
{
    (void)base_freq;
    (void)harmonics;
    (void)intensities;
    (void)num_harmonics;
    (void)duration_ms;
}

void termio_render_graphics(void)
{
}
void termio_show_welcome(const char *name, const char *version)
{
    if (!g_sdl_enabled || !g_renderer || !g_font)
        return;

    /* Welcome text lines (with proper UTF-8 support) */
    const char *lines[] = {
        name,
        version,
        "© 2026 Meltingcaps.com",
        "",
        "[Press any key to continue]"};
    int num_lines = 5;

    /* Starting row for typewriter effect (vertically centered) */
    int start_row = 8;

    /* Track which line and character we're currently typing */
    int current_line = 0;
    int current_char = 0;
    int display_complete = 0;

    /* Animation timing */
    Uint32 last_char_time = SDL_GetTicks();
    Uint32 last_blink_time = SDL_GetTicks();
    int blink_state = 0;
    Uint32 char_delay_ms = 40; /* Time between character reveals */

    int done = 0;

    while (!done)
    {
        Uint32 now = SDL_GetTicks();

        /* If not yet complete, advance character animation */
        if (!display_complete && (now - last_char_time >= char_delay_ms))
        {
            last_char_time = now;
            current_char++;

            /* Check if we've displayed all chars in current line */
            if (current_char > (int)strlen(lines[current_line]))
            {
                current_char = 0;
                current_line++;

                /* Skip empty line briefly, then move to next real line */
                if (current_line >= num_lines)
                {
                    display_complete = 1;
                }
            }
        }

        /* Blink cursor effect */
        if (now - last_blink_time > 500)
        {
            blink_state = !blink_state;
            last_blink_time = now;
        }

        /* Render using SDL directly (bypass g_screen for UTF-8 support) */
        SDL_SetRenderDrawColor(g_renderer, g_bg_color.r, g_bg_color.g, g_bg_color.b, 255);
        SDL_RenderClear(g_renderer);

        int renderer_w, renderer_h;
        SDL_GetRendererOutputSize(g_renderer, &renderer_w, &renderer_h);

        /* Draw lines that have been typed so far */
        for (int i = 0; i < current_line; i++)
        {
            const char *line = lines[i];
            int y = (int)((start_row + i) * g_char_height);

            SDL_Color line_color = g_palette[0]; /* Normal color */
            if (i == 0)
                line_color = g_palette[COL_KEYWORD]; /* Name in pink */
            else if (i == 2)
                line_color = g_palette[COL_COMMENT]; /* Copyright in gray */

            /* Render full line using SDL_ttf UTF-8 support */
            SDL_Surface *surf = TTF_RenderUTF8_Blended(g_font, line, line_color);
            if (surf)
            {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                if (tex)
                {
                    int w = surf->w;
                    int x = (renderer_w - w) / 2; /* Center horizontally */
                    SDL_Rect rect = {x, y, w, g_char_height};
                    SDL_RenderCopy(g_renderer, tex, NULL, &rect);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }

        /* Draw current line being typed */
        if (!display_complete && current_line < num_lines)
        {
            const char *line = lines[current_line];
            int y = (int)((start_row + current_line) * g_char_height);

            SDL_Color line_color = g_palette[0];
            if (current_line == 0)
                line_color = g_palette[COL_KEYWORD];
            else if (current_line == 2)
                line_color = g_palette[COL_COMMENT];

            /* Create partial string for current typing progress */
            char partial[256];
            int copy_len = (current_char < (int)sizeof(partial) - 1) ? current_char : (int)sizeof(partial) - 1;
            strncpy(partial, line, copy_len);
            partial[copy_len] = 0;

            /* Render partial line */
            if (copy_len > 0)
            {
                SDL_Surface *surf = TTF_RenderUTF8_Blended(g_font, partial, line_color);
                if (surf)
                {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                    if (tex)
                    {
                        int w = surf->w;
                        int x = (renderer_w - (int)strlen(line) * g_char_width / 2) / 2; /* Approximate centering */
                        SDL_Rect rect = {x, y, w, g_char_height};
                        SDL_RenderCopy(g_renderer, tex, NULL, &rect);
                        SDL_DestroyTexture(tex);
                    }
                    SDL_FreeSurface(surf);
                }
            }

            /* Draw blinking cursor */
            if (blink_state && current_char < (int)strlen(line))
            {
                SDL_Surface *surf = TTF_RenderUTF8_Blended(g_font, partial, line_color);
                if (surf)
                {
                    int cursor_x = (renderer_w - (int)strlen(line) * g_char_width / 2) / 2 + surf->w + 2;
                    SDL_Rect cursor = {cursor_x, y + (int)(g_char_height * 0.7), 2, (int)(g_char_height * 0.25)};
                    SDL_SetRenderDrawColor(g_renderer, line_color.r, line_color.g, line_color.b, 255);
                    SDL_RenderFillRect(g_renderer, &cursor);
                    SDL_FreeSurface(surf);
                }
            }
        }

        SDL_RenderPresent(g_renderer);

        /* Handle input */
        SDL_Event event;
        if (SDL_WaitEventTimeout(&event, 50))
        {
            if (event.type == SDL_QUIT)
                exit(0);
            if (event.type == SDL_KEYDOWN)
            {
                /* Any key dismisses / skips to final state */
                if (!display_complete)
                {
                    /* Skip animation, show everything immediately */
                    display_complete = 1;
                    current_line = num_lines - 1;
                    current_char = (int)strlen(lines[current_line]);
                }
                else
                {
                    /* Already complete, exit welcome screen */
                    done = 1;
                }
            }
        }
    }

    /* Clear screen and show Ok prompt */
    termio_clear();
    g_cursor_row = 0;
    g_cursor_col = 0;
}

/* ------------------------------------------------------------------ */
/* Syntax highlighting helpers                                          */
/* ------------------------------------------------------------------ */

void termio_set_write_color(int color_idx)
{
    if (!g_sdl_enabled)
        return;
    if (color_idx < 0 || color_idx > 6)
        color_idx = 0;
    g_write_color = (Uint8)color_idx;
}

/* Basic++ keywords that should appear in COL_KEYWORD */
static const char *s_keywords[] = {
    "PROCEDURE", "END", "CLASS", "NEW", "RETURN",
    "IF", "THEN", "ELSE", "ELSIF", "ENDIF",
    "FOR", "TO", "STEP", "NEXT",
    "WHILE", "WEND", "DO", "UNTIL", "LOOP",
    "PRINT", "INPUT", "LET", "DIM",
    "READ", "DATA", "GOTO", "GOSUB",
    "ON", "STOP", "REM", "DEF",
    "AND", "OR", "NOT", "MOD",
    "TRUE", "FALSE", "SELF",
    NULL};

static int is_keyword(const char *start, int len)
{
    for (int i = 0; s_keywords[i]; i++)
    {
        int klen = (int)strlen(s_keywords[i]);
        if (klen == len && strncasecmp(start, s_keywords[i], len) == 0)
            return 1;
    }
    return 0;
}

void termio_write_highlighted(const char *line)
{
    if (!line)
        return;
    if (!g_sdl_enabled)
    {
        /* plain terminal fallback */
        termio_write(line);
        termio_write_char('\n');
        return;
    }

    /* Track whether the previous keyword was PROCEDURE or CLASS
       so the next identifier gets COL_PROCNAME */
    int next_is_procname = 0;

    const char *p = line;
    while (*p)
    {
        /* Comment: ! to end of line */
        if (*p == '!')
        {
            termio_set_write_color(COL_COMMENT);
            while (*p)
                termio_write_char(*p++);
            break;
        }

        /* String literal */
        if (*p == '"')
        {
            termio_set_write_color(COL_STRING);
            termio_write_char(*p++);
            while (*p && *p != '"')
                termio_write_char(*p++);
            if (*p == '"')
                termio_write_char(*p++);
            termio_set_write_color(COL_NORMAL);
            continue;
        }

        /* Number: digit or leading dot-digit */
        if ((*p >= '0' && *p <= '9') ||
            (*p == '.' && *(p + 1) >= '0' && *(p + 1) <= '9'))
        {
            termio_set_write_color(COL_NUMBER);
            while ((*p >= '0' && *p <= '9') || *p == '.' || *p == 'e' || *p == 'E' ||
                   (((*p == '+' || *p == '-') && (*(p - 1) == 'e' || *(p - 1) == 'E'))))
                termio_write_char(*p++);
            termio_set_write_color(COL_NORMAL);
            continue;
        }

        /* Identifier or keyword */
        if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || *p == '_')
        {
            const char *start = p;
            while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
                   (*p >= '0' && *p <= '9') || *p == '_')
                p++;
            int len = (int)(p - start);

            int color;
            if (next_is_procname)
            {
                color = COL_PROCNAME;
                next_is_procname = 0;
            }
            else if (is_keyword(start, len))
            {
                color = COL_KEYWORD;
                /* peek: if PROCEDURE or CLASS, next ident is a proc name */
                if (strncasecmp(start, "PROCEDURE", 9) == 0 ||
                    strncasecmp(start, "CLASS", 5) == 0)
                    next_is_procname = 1;
            }
            else
            {
                color = COL_NORMAL;
            }

            termio_set_write_color(color);
            for (const char *q = start; q < p; q++)
                termio_write_char(*q);
            termio_set_write_color(COL_NORMAL);
            continue;
        }

        /* Operators / punctuation */
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '^' ||
            *p == '=' || *p == '<' || *p == '>' || *p == '(' || *p == ')' ||
            *p == '[' || *p == ']' || *p == ',' || *p == ':' || *p == ';')
        {
            termio_set_write_color(COL_OPERATOR);
            termio_write_char(*p++);
            termio_set_write_color(COL_NORMAL);
            continue;
        }

        /* Anything else — whitespace, dots, etc. */
        termio_set_write_color(COL_NORMAL);
        termio_write_char(*p++);
    }

    /* End line */
    termio_set_write_color(COL_NORMAL);
    termio_write_char('\n');
}