#include <SDL2/SDL.h>
#include "video_backend.h"

static int g_sdl_enabled = 0;
static SDL_Renderer *g_renderer = NULL;

/* CoCo-style 9-color palette (RGB) */
static const unsigned char g_coco_palette[16][3] = {
    {0, 0, 0},       /* 0 black */
    {0, 255, 0},     /* 1 green */
    {255, 255, 0},   /* 2 yellow */
    {0, 0, 255},     /* 3 blue */
    {255, 0, 0},     /* 4 red */
    {255, 255, 200}, /* 5 buff */
    {0, 255, 255},   /* 6 cyan */
    {255, 0, 255},   /* 7 magenta */
    {255, 128, 0},   /* 8 orange */
    {128, 128, 128}, /* 9 gray */
    {0, 128, 0},     /* 10 dark green */
    {128, 128, 0},   /* 11 olive */
    {0, 0, 128},     /* 12 dark blue */
    {128, 0, 0},     /* 13 dark red */
    {128, 128, 64},  /* 14 */
    {128, 64, 128},  /* 15 */
};

void render_video_memory(void)
{
    if (!g_sdl_enabled || !g_renderer)
        return;
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    int pixel_w = 2, pixel_h = 2;
    for (int y = 0; y < video_memory.height; y++)
    {
        for (int x = 0; x < video_memory.width; x++)
        {
            int c = video_memory.pixels[y][x];
            if (c < 0 || c >= 16)
                c = 0;
            unsigned char r = g_coco_palette[c][0];
            unsigned char g = g_coco_palette[c][1];
            unsigned char b = g_coco_palette[c][2];
            SDL_SetRenderDrawColor(g_renderer, r, g, b, 255);
            SDL_Rect rect = {x * pixel_w, y * pixel_h, pixel_w, pixel_h};
            SDL_RenderFillRect(g_renderer, &rect);
        }
    }
    SDL_RenderPresent(g_renderer);
}

#ifdef USE_SDL

#include "termio.h"
#include "video_backend.h"
#include "videomem.h"
#include "executor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#define TERM_COLS 80
#define TERM_ROWS 24
#define SCROLLBACK_MAX 2000

typedef struct
{
    char *cells;
} Line;

/* Global state (g_sdl_enabled, g_renderer at file scope) */
static SDL_Window *g_window = NULL;
static TTF_Font *g_font = NULL;
static int g_cell_w = 8, g_cell_h = 16;
static int g_cols = TERM_COLS, g_rows = TERM_ROWS;
static int g_scale = 1, g_border = 1;

static Line g_lines[SCROLLBACK_MAX];
static int g_line_start = 0, g_line_count = 0;
static int g_col_start = 0; /* Horizontal scroll offset */
static int g_cur_line = 0, g_cur_col = 0;
static int g_scroll_line = 0; /* Position of browse cursor in scrollback for scrolling */
static int g_view_start = 0;  /* Top line of viewport in scrollback */
static int g_browse_line = 0; /* Currently selected line for editing (during scrollback browse) */
static int g_running = 1;

static char g_input_buf[1024];
static int g_input_len = 0, g_input_cursor = 0;
static int g_input_active = 0, g_input_prompt_line = 0, g_input_prompt_col = 0;

static int g_edit_mode = 0; /* 1 if in LINE EDIT mode */
static int g_edit_line_num = 0;

static int g_show_browse_highlight = 0;

/* Video memory tracking */
static int g_video_mem_line = 0; /* Current row in video memory (0-23) */
static int g_video_mem_col = 0;  /* Current col in video memory (0-79) */

/* Blinking cursor state */
static Uint32 g_last_blink_time = 0;
static int g_cursor_visible = 1;
static const int BLINK_INTERVAL = 500; /* 500ms on/off */

/* INKEY$ key buffer */
static int g_polled_key = -1;

/* Text color state: 0=black, 1=white */
static int g_text_fg = 1; /* white foreground (default) */
static int g_text_bg = 0; /* black background (default) */

/* Helper: find TTF font path */
static const char *choose_font_path(void)
{
    const char *env = getenv("TRS80_FONT");
    struct stat st;

    if (env && stat(env, &st) == 0)
        return env;
#ifdef __APPLE__
    if (stat("/System/Library/Fonts/SFNSMono.ttf", &st) == 0)
        return "/System/Library/Fonts/SFNSMono.ttf";
    if (stat("/Library/Fonts/Menlo.ttc", &st) == 0)
        return "/Library/Fonts/Menlo.ttc";
#endif
    if (stat("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", &st) == 0)
        return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    if (stat("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", &st) == 0)
        return "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
    return NULL;
}

/* Map video memory byte to UTF-8 glyph for rendering */
static const char *glyph_for_byte(unsigned char c, char *buf)
{
    switch (c)
    {
    case 0xDB:                 /* Full block */
        return "\xE2\x96\x88"; /* U+2588 */
    case 0xFE:                 /* Black square */
        return "\xE2\x96\xA0"; /* U+25A0 */
    case 0xB2:                 /* Dark shade */
        return "\xE2\x96\x93"; /* U+2593 */
    case 0xB1:                 /* Medium shade */
        return "\xE2\x96\x92"; /* U+2592 */
    case 0xB0:                 /* Light shade */
        return "\xE2\x96\x91"; /* U+2591 */
    default:
        buf[0] = (c < 128) ? (char)c : '?';
        buf[1] = '\0';
        return buf;
    }
}

static int is_solid_block(unsigned char c)
{
    return (c == 0xDB || c == 0xFE || c == 0xB2 || c == 0xB1 || c == 0xB0);
}

/* Initialize line storage */
static void init_lines(void)
{
    for (int i = 0; i < SCROLLBACK_MAX; i++)
    {
        g_lines[i].cells = (char *)malloc(g_cols);
        memset(g_lines[i].cells, ' ', g_cols);
    }
    g_line_count = 1;
    g_cur_line = 0;
    g_cur_col = 0;
    g_scroll_line = 0; /* Start browsing at the first line */
    g_view_start = 0;
}

/* Ensure line exists */
static void ensure_line(int idx)
{
    if (idx < 0)
        return;
    while (idx >= g_line_count)
    {
        int real = (g_line_start + g_line_count) % SCROLLBACK_MAX;
        memset(g_lines[real].cells, ' ', g_cols);
        g_line_count++;
        if (g_line_count > SCROLLBACK_MAX)
        {
            g_line_start = (g_line_start + 1) % SCROLLBACK_MAX;
            g_line_count = SCROLLBACK_MAX;
            if (g_cur_line > 0)
                g_cur_line--;
            if (g_scroll_line > 0)
                g_scroll_line--;
            if (g_view_start > 0)
                g_view_start--;
        }
    }
}

/* Put character at position */
static void put_char_at(int line, int col, char c)
{
    if (line < 0 || col < 0 || col >= g_cols)
        return;
    ensure_line(line);
    int real = (g_line_start + line) % SCROLLBACK_MAX;
    g_lines[real].cells[col] = c;
}

/* Advance to next line */
static void newline_advance(void)
{
    int was_at_bottom = (g_scroll_line >= g_cur_line);
    g_cur_line++;
    g_cur_col = 0;
    ensure_line(g_cur_line);
    if (was_at_bottom)
    {
        g_scroll_line = g_cur_line; /* Keep browse cursor at latest line */
        if (g_cur_line >= g_rows)
            g_view_start = g_cur_line - g_rows + 1;
        else
            g_view_start = 0;
    }
}

/* Render screen */
static void render_text(void)
{
    if (!g_sdl_enabled || !g_renderer)
        return;

    if (video_graphics_active())
    {
        int gw = video_memory.width * 2;
        int gh = video_memory.height * 2;
        SDL_SetWindowSize(g_window, gw, gh);
        render_video_memory();
        return;
    }

    /* Set background color based on state */
    if (g_text_bg == 1)
    {
        SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255); /* white background */
    }
    else
    {
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255); /* black background */
    }
    SDL_RenderClear(g_renderer);

    /* Border around 24 rows of content */
    SDL_Rect border = {g_border - 1, g_border - 1,
                       (g_cols + 1) * g_cell_w,
                       (g_rows + 1) * g_cell_h};
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(g_renderer, &border);

    /* Separator line before preview/edit row (row 24) */
    int separator_y = g_border * g_cell_h + g_rows * g_cell_h;
    SDL_RenderDrawLine(g_renderer, g_border * g_cell_w, separator_y,
                       g_border * g_cell_w + g_cols * g_cell_w, separator_y);

    /* Calculate foreground color from state */
    SDL_Color fg;
    if (g_text_fg == 1)
    {
        fg.r = 255;
        fg.g = 255;
        fg.b = 255;
        fg.a = 255; /* white */
    }
    else
    {
        fg.r = 0;
        fg.g = 0;
        fg.b = 0;
        fg.a = 255; /* black */
    }

    /* Calculate viewport offset (used for both scrollback and input rendering) */
    int visible_lines = (g_line_count > g_rows) ? g_rows : g_line_count;
    int max_view_start = (g_line_count > g_rows) ? (g_line_count - g_rows) : 0;
    if (g_view_start < 0)
        g_view_start = 0;
    if (g_view_start > max_view_start)
        g_view_start = max_view_start;
    int start_offset = g_view_start;

    /* Render scrollback buffer */
    for (int row = 0; row < visible_lines; row++)
    {
        int line_idx = (g_line_start + start_offset + row) % SCROLLBACK_MAX;

        if (g_show_browse_highlight)
        {
            int absolute_line = start_offset + row;
            int is_browse_line = (g_input_active && absolute_line == g_browse_line);
            if (is_browse_line)
            {
                SDL_Rect highlight = {g_border * g_cell_w,
                                      g_border * g_cell_h + row * g_cell_h,
                                      g_cols * g_cell_w, g_cell_h};
                SDL_SetRenderDrawColor(g_renderer, 100, 150, 255, 100); /* Translucent blue */
                SDL_RenderFillRect(g_renderer, &highlight);
            }
        }

        for (int col = 0; col < g_cols; col++)
        {
            int source_col = col + g_col_start;
            if (source_col >= g_cols)
                continue;
            char c = g_lines[line_idx].cells[source_col];
            if (c != ' ')
            {
                if ((unsigned char)c == 233) /* Ball: draw round white circle */
                {
                    int cx = g_border * g_cell_w + col * g_cell_w + g_cell_w / 2;
                    int cy = g_border * g_cell_h + row * g_cell_h + g_cell_h / 2;
                    int radius = (g_cell_w < g_cell_h ? g_cell_w : g_cell_h) / 2 - 2;
                    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
                    for (int y = -radius; y <= radius; y++)
                    {
                        for (int x = -radius; x <= radius; x++)
                        {
                            if (x * x + y * y <= radius * radius)
                            {
                                SDL_RenderDrawPoint(g_renderer, cx + x, cy + y);
                            }
                        }
                    }
                    continue;
                }
                if (is_solid_block((unsigned char)c))
                {
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_SetRenderDrawColor(g_renderer, fg.r, fg.g, fg.b, fg.a);
                    SDL_RenderFillRect(g_renderer, &dst);
                    continue;
                }
                char str[4];
                const char *glyph = glyph_for_byte((unsigned char)c, str);
                SDL_Surface *surf = TTF_RenderText_Solid(g_font, glyph, fg);
                if (surf)
                {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_RenderCopy(g_renderer, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
        }
    }

    /* Render preview/edit line at row 24 when browsing */
    if (g_input_active && g_browse_line >= 0 && g_browse_line < g_line_count)
    {
        int preview_row = g_rows; /* Row 24 (0-indexed) */
        int line_idx = (g_line_start + g_browse_line) % SCROLLBACK_MAX;

        /* Highlight the preview line background */
        SDL_Rect preview_bg = {g_border * g_cell_w,
                               g_border * g_cell_h + preview_row * g_cell_h,
                               g_cols * g_cell_w, g_cell_h};
        /* Use same background color as main display */
        if (g_text_bg == 1)
        {
            SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255); /* Light gray on white background */
        }
        else
        {
            SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255); /* Dark gray on black background */
        }
        SDL_RenderFillRect(g_renderer, &preview_bg);

        /* Render the browsed line content */
        for (int col = 0; col < g_cols; col++)
        {
            char c = g_lines[line_idx].cells[col];
            if (c != ' ')
            {
                if (is_solid_block((unsigned char)c))
                {
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + preview_row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_SetRenderDrawColor(g_renderer, fg.r, fg.g, fg.b, fg.a);
                    SDL_RenderFillRect(g_renderer, &dst);
                    continue;
                }
                char str[4];
                const char *glyph = glyph_for_byte((unsigned char)c, str);
                SDL_Surface *surf = TTF_RenderText_Solid(g_font, glyph, fg);
                if (surf)
                {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + preview_row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_RenderCopy(g_renderer, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
        }
    }

    /* Calculate viewport offset for input rendering */

    /* Render active input line */
    if (g_input_active && (g_input_len > 0 || g_edit_mode))
    {
        int input_row = g_input_prompt_line;
        int input_col = g_input_prompt_col;

        /* In EDIT mode at preview line (row 24), render directly without viewport offset */
        int use_viewport_offset = !(g_edit_mode && input_row == g_rows);

        /* In EDIT mode, render line number first */
        if (g_edit_mode)
        {
            char line_num_str[16];
            snprintf(line_num_str, sizeof(line_num_str), "%d ", g_edit_line_num);
            for (const char *p = line_num_str; *p; p++)
            {
                int row = input_row;
                int col = input_col + (p - line_num_str);
                int viewport_row = use_viewport_offset ? (row - start_offset) : row;
                if (viewport_row >= 0 && viewport_row <= g_rows && col < g_cols)
                {
                    if (is_solid_block((unsigned char)*p))
                    {
                        SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                        g_border * g_cell_h + viewport_row * g_cell_h,
                                        g_cell_w, g_cell_h};
                        SDL_SetRenderDrawColor(g_renderer, fg.r, fg.g, fg.b, fg.a);
                        SDL_RenderFillRect(g_renderer, &dst);
                        continue;
                    }
                    char str[4];
                    const char *glyph = glyph_for_byte((unsigned char)*p, str);
                    SDL_Surface *surf = TTF_RenderText_Solid(g_font, glyph, fg);
                    if (surf)
                    {
                        SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                        SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                        g_border * g_cell_h + viewport_row * g_cell_h,
                                        g_cell_w, g_cell_h};
                        SDL_RenderCopy(g_renderer, tex, NULL, &dst);
                        SDL_DestroyTexture(tex);
                        SDL_FreeSurface(surf);
                    }
                }
            }
            input_col += strlen(line_num_str);
        }

        int chars_on_first_line = g_cols - input_col; /* How many chars fit on first line */

        for (int i = 0; i < g_input_len; i++)
        {
            char c = g_input_buf[i];
            int row, col;

            if (i < chars_on_first_line)
            {
                /* First line of input (after line number in EDIT mode) */
                row = input_row;
                col = input_col + i;
            }
            else
            {
                /* Subsequent lines (wrapped, starting from column 0) */
                int chars_after_first = i - chars_on_first_line;
                row = input_row + 1 + (chars_after_first / g_cols);
                col = chars_after_first % g_cols;
            }

            /* Convert absolute row to viewport row */
            int viewport_row = use_viewport_offset ? (row - start_offset) : row;
            if (viewport_row >= 0 && viewport_row <= g_rows && col >= 0 && col < g_cols)
            {
                if (is_solid_block((unsigned char)c))
                {
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + viewport_row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_SetRenderDrawColor(g_renderer, fg.r, fg.g, fg.b, fg.a);
                    SDL_RenderFillRect(g_renderer, &dst);
                    continue;
                }
                char str[4];
                const char *glyph = glyph_for_byte((unsigned char)c, str);
                SDL_Surface *surf = TTF_RenderText_Solid(g_font, glyph, fg);
                if (surf)
                {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
                    SDL_Rect dst = {g_border * g_cell_w + col * g_cell_w,
                                    g_border * g_cell_h + viewport_row * g_cell_h,
                                    g_cell_w, g_cell_h};
                    SDL_RenderCopy(g_renderer, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
        }
    }

    /* Render blinking cursor when input is active */
    if (g_input_active)
    {
        int input_row = g_input_prompt_line;
        int input_col = g_input_prompt_col;

        /* In EDIT mode at preview line (row 24), render directly without viewport offset */
        int use_viewport_offset = !(g_edit_mode && input_row == g_rows);

        /* In EDIT mode, account for line number offset */
        if (g_edit_mode)
        {
            char line_num_str[16];
            snprintf(line_num_str, sizeof(line_num_str), "%d ", g_edit_line_num);
            input_col += strlen(line_num_str);
        }

        int chars_on_first_line = g_cols - input_col;

        int cursor_row, cursor_col;
        if (g_input_cursor < chars_on_first_line)
        {
            cursor_row = g_input_prompt_line;
            cursor_col = input_col + g_input_cursor;
        }
        else
        {
            int chars_after_first = g_input_cursor - chars_on_first_line;
            cursor_row = g_input_prompt_line + 1 + (chars_after_first / g_cols);
            cursor_col = chars_after_first % g_cols;
        }

        /* Convert absolute cursor_row to viewport row */
        int viewport_row = use_viewport_offset ? (cursor_row - start_offset) : cursor_row;

        /* Update blink state */
        Uint32 now = SDL_GetTicks();
        if (now - g_last_blink_time >= BLINK_INTERVAL)
        {
            g_cursor_visible = !g_cursor_visible;
            g_last_blink_time = now;
        }

        /* Render blinking cursor */
        if (g_cursor_visible && viewport_row >= 0 && viewport_row <= g_rows)
        {
            if (g_edit_mode)
            {
                /* Half-height cursor for LINE EDIT mode */
                SDL_Rect cursor = {g_border * g_cell_w + cursor_col * g_cell_w,
                                   g_border * g_cell_h + viewport_row * g_cell_h + g_cell_h / 2,
                                   g_cell_w, g_cell_h / 2};
                SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255); /* Yellow half cursor */
                SDL_RenderFillRect(g_renderer, &cursor);
            }
            else
            {
                /* Full-height block cursor in normal mode */
                SDL_Rect cursor = {g_border * g_cell_w + cursor_col * g_cell_w,
                                   g_border * g_cell_h + viewport_row * g_cell_h,
                                   g_cell_w, g_cell_h};
                SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255); /* Light gray block */
                SDL_RenderFillRect(g_renderer, &cursor);
            }
        }
    }

    SDL_RenderPresent(g_renderer);
}

void termio_set_colors(int fg, int bg)
{
    g_text_fg = (fg == 1) ? 1 : 0;
    g_text_bg = (bg == 1) ? 1 : 0;
}

/* ---- PUBLIC API ---- */

int termio_init(int cols, int rows, int scale)
{
    g_cols = cols > 0 ? cols : TERM_COLS;
    g_rows = rows > 0 ? rows : TERM_ROWS;
    g_scale = scale > 0 ? scale : 1;

    /* Check if SDL should be disabled via environment variable */
    const char *no_sdl = getenv("BASIC_NO_SDL");
    if (no_sdl && *no_sdl)
    {
        /* Text-only mode requested, skip SDL initialization */
        g_sdl_enabled = 0;
        return 1;
    }

    /* Try SDL init, fail gracefully */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0)
        return 1;
    if (TTF_Init() != 0)
    {
        SDL_Quit();
        return 1;
    }

    const char *font_path = choose_font_path();
    if (!font_path)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int font_size = 16 * g_scale;
    g_font = TTF_OpenFont(font_path, font_size);
    if (!g_font)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_SizeText(g_font, "W", &g_cell_w, &g_cell_h);

    /* Window has 24 rows for content + 1 row for preview/edit line */
    int window_rows = g_rows + 1;
    int width = (g_cols + 2 * g_border) * g_cell_w;
    int height = (window_rows + 2 * g_border) * g_cell_h;

    g_window = SDL_CreateWindow("TRS-80 BASIC", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, SDL_WINDOW_SHOWN);
    if (!g_window)
    {
        TTF_CloseFont(g_font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer)
    {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        TTF_CloseFont(g_font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    g_sdl_enabled = 1;
    const char *highlight_env = getenv("TRS80_BROWSE_HIGHLIGHT");
    if (highlight_env && highlight_env[0] != '\0')
    {
        char c = (char)tolower((unsigned char)highlight_env[0]);
        g_show_browse_highlight = (c == '1' || c == 'y' || c == 't');
    }
    SDL_StartTextInput();
    init_lines();
    render_text();
    return 1;
}

void termio_shutdown(void)
{
    if (!g_sdl_enabled)
        return;

    SDL_StopTextInput();
    if (g_font)
        TTF_CloseFont(g_font);
    if (g_renderer)
        SDL_DestroyRenderer(g_renderer);
    if (g_window)
        SDL_DestroyWindow(g_window);
    TTF_Quit();
    SDL_Quit();

    for (int i = 0; i < SCROLLBACK_MAX; i++)
    {
        free(g_lines[i].cells);
        g_lines[i].cells = NULL;
    }
    g_sdl_enabled = 0;
}

void termio_clear(void)
{
    if (!g_sdl_enabled)
    {
        printf("\033[2J\033[H");
        fflush(stdout);
        return;
    }

    for (int i = 0; i < g_line_count; i++)
    {
        int real = (g_line_start + i) % SCROLLBACK_MAX;
        memset(g_lines[real].cells, ' ', g_cols);
    }
    g_line_count = 1;
    g_cur_line = 0;
    g_cur_col = 0;
    g_scroll_line = 0;
    g_view_start = 0;

    /* Reset video memory position tracking (but don't clear the actual video memory bytes) */
    g_video_mem_line = 0;
    g_video_mem_col = 0;

    render_text();
}

void termio_set_cursor(int row, int col)
{
    if (!g_sdl_enabled)
    {
        printf("\033[%d;%dH", row, col);
        fflush(stdout);
        return;
    }

    /* Convert from 1-based to 0-based */
    row--;
    col--;

    /* Clamp column to valid range */
    if (row < 0)
        row = 0;
    if (col < 0)
        col = 0;
    if (col >= g_cols)
        col = g_cols - 1;

    /* Expand line buffer if necessary */
    while (row >= g_line_count)
    {
        /* Add a new blank line */
        int real = (g_line_start + g_line_count) % SCROLLBACK_MAX;
        memset(g_lines[real].cells, ' ', g_cols);
        g_line_count++;
    }

    g_cur_line = row;
    g_cur_col = col;
}

void termio_put_char_at(int row, int col, char c)
{
    if (!g_sdl_enabled)
    {
        printf("\033[s\033[%d;%dH%c\033[u", row + 1, col + 1, c);
        fflush(stdout);
        return;
    }

    if (row < 0 || col < 0 || col >= g_cols || row >= TERM_ROWS)
    {
        return;
    }

    ensure_line(row);
    put_char_at(row, col, c);
    render_text();
}

void termio_write_char(char c)
{
    if (!g_sdl_enabled)
    {
        fputc(c, stdout);
        fflush(stdout);
        return;
    }

    if (c == '\n')
    {
        newline_advance();

        /* Update video memory: if we've written past row 23, scroll video memory */
        if (g_video_mem_line >= TERM_ROWS)
        {
            videomem_scroll_up();
            /* Keep video memory at rows 0-23 */
            g_video_mem_line = TERM_ROWS - 1;
            g_video_mem_col = 0;
        }
        else
        {
            g_video_mem_line++;
            g_video_mem_col = 0;
        }

        render_text();
        return;
    }
    if (c == '\r')
    {
        g_cur_col = 0;
        g_video_mem_col = 0;
        return;
    }

    put_char_at(g_cur_line, g_cur_col, c);

    /* Update video memory with the character */
    if (g_video_mem_line < TERM_ROWS && g_video_mem_col < 80)
    {
        videomem_write_char(g_video_mem_line, g_video_mem_col, (unsigned char)c);
    }

    g_cur_col++;
    g_video_mem_col++;

    if (g_cur_col > g_cols)
        newline_advance();
}

void termio_write(const char *str)
{
    if (!str)
        return;
    if (!g_sdl_enabled)
    {
        fputs(str, stdout);
        fflush(stdout);
        return;
    }
    while (*str)
        termio_write_char(*str++);
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
    if (g_sdl_enabled)
        render_text();
    else
        fflush(stdout);
}

/* LINE EDIT mode - edit a line with full cursor control */
int termio_lineedit(int line_num, char *buf, int maxlen)
{
    if (!buf || maxlen <= 0)
        return -1;

    if (!g_sdl_enabled)
    {
        /* Fallback: just use readline */
        if (fgets(buf, maxlen, stdin) == NULL)
            return -1;
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';
        for (size_t i = 0; i < len; i++)
        {
            unsigned char ch = (unsigned char)buf[i];
            buf[i] = (char)toupper(ch);
        }
        return (int)len;
    }

    /* SDL LINE EDIT mode */
    memset(g_input_buf, 0, sizeof(g_input_buf));
    strncpy(g_input_buf, buf, sizeof(g_input_buf) - 1);
    g_input_len = strlen(g_input_buf);
    g_input_cursor = 0; /* Start at beginning of line in edit mode */
    g_input_active = 1;
    g_edit_mode = 1;
    g_edit_line_num = line_num;
    /* Position edit at the preview line (row 24) */
    g_input_prompt_line = g_rows; /* Row 24 (0-indexed as g_rows = 24) */
    g_input_prompt_col = 0;       /* Line number will be shown at col 0 */

    /* Don't write to scrollback - render_text will display the line */

    while (g_running && g_input_active)
    {
        /* Check for Ctrl-C (interrupt signal) */
        if (executor_check_interrupt())
        {
            g_input_active = 0;
            g_input_len = 0;
            return -1;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                g_running = 0;
                g_input_active = 0;
            }
            else if (event.type == SDL_TEXTINPUT && g_input_len < (int)sizeof(g_input_buf) - 1)
            {
                const char *text = event.text.text;
                for (int i = 0; text[i]; i++)
                {
                    unsigned char ch = (unsigned char)text[i];
                    if (ch >= 32 && ch < 127)
                    {
                        memmove(&g_input_buf[g_input_cursor + 1], &g_input_buf[g_input_cursor],
                                g_input_len - g_input_cursor);
                        g_input_buf[g_input_cursor] = (char)toupper(ch);
                        g_input_len++;
                        g_input_cursor++;
                    }
                }
                render_text();
            }
            else if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Keymod mod = event.key.keysym.mod;

                /* Ctrl+C interrupt */
                if (key == SDLK_c && (mod & KMOD_CTRL))
                {
                    executor_trigger_interrupt();
                    termio_write("^C\n");
                    g_input_active = 0;
                    g_input_len = 0;
                    return -1;
                }

                if (key == SDLK_b && (mod & KMOD_CTRL))
                {
                    g_show_browse_highlight = !g_show_browse_highlight;
                    render_text();
                }
                /* Handle Cmd+V (macOS) or Ctrl+V (other) for paste */
                else if (key == SDLK_v && ((mod & KMOD_GUI) || (mod & KMOD_CTRL)))
                {
                    char *clipboard = SDL_GetClipboardText();
                    if (clipboard && clipboard[0] != '\0')
                    {
                        /* In edit mode, only paste first line if multi-line */
                        for (int i = 0; clipboard[i] && g_input_len < (int)sizeof(g_input_buf) - 1; i++)
                        {
                            unsigned char ch = (unsigned char)clipboard[i];
                            if (ch == '\n' || ch == '\r')
                                break; /* Stop at newline */
                            if (ch >= 32 && ch < 127)
                            {
                                memmove(&g_input_buf[g_input_cursor + 1], &g_input_buf[g_input_cursor],
                                        g_input_len - g_input_cursor);
                                g_input_buf[g_input_cursor] = (char)toupper(ch);
                                g_input_len++;
                                g_input_cursor++;
                            }
                        }
                        SDL_free(clipboard);
                        render_text();
                    }
                }
                else if (key == SDLK_RETURN)
                {
                    g_input_active = 0;
                    g_edit_mode = 0;
                }
                else if (key == SDLK_ESCAPE)
                {
                    memset(g_input_buf, 0, sizeof(g_input_buf));
                    g_input_len = 0;
                    g_input_active = 0;
                    g_edit_mode = 0;
                }
                else if (key == SDLK_BACKSPACE && g_input_cursor > 0)
                {
                    g_input_cursor--;
                    if (g_input_len > 0 && g_input_cursor < g_input_len)
                    {
                        memmove(&g_input_buf[g_input_cursor], &g_input_buf[g_input_cursor + 1],
                                g_input_len - g_input_cursor - 1);
                        g_input_len--;
                        g_input_buf[g_input_len] = '\0';
                    }
                    render_text();
                }
                else if (key == SDLK_DELETE && g_input_cursor < g_input_len)
                {
                    if (g_input_len > 0 && g_input_cursor < g_input_len)
                    {
                        memmove(&g_input_buf[g_input_cursor], &g_input_buf[g_input_cursor + 1],
                                g_input_len - g_input_cursor - 1);
                        g_input_len--;
                        g_input_buf[g_input_len] = '\0';
                    }
                    render_text();
                }
                else if (key == SDLK_LEFT && g_input_cursor > 0)
                {
                    g_input_cursor--;
                    render_text();
                }
                else if (key == SDLK_RIGHT && g_input_cursor <= g_input_len)
                {
                    if (g_input_cursor < (int)sizeof(g_input_buf) - 1)
                        g_input_cursor++;
                    render_text();
                }
                else if (key == SDLK_HOME)
                {
                    g_input_cursor = 0;
                    render_text();
                }
                else if (key == SDLK_END)
                {
                    g_input_cursor = g_input_len;
                    render_text();
                }
            }
        }
        render_text();
    }

    int len = g_input_len < maxlen ? g_input_len : maxlen - 1;
    memcpy(buf, g_input_buf, len);
    buf[len] = '\0';

    g_edit_mode = 0;
    return g_running ? len : -1;
}

int termio_readline(char *buf, int maxlen)
{
    if (!buf || maxlen <= 0)
        return -1;

    if (!g_sdl_enabled)
    {
        /* Fallback: stdio mode */
        if (fgets(buf, maxlen, stdin) == NULL)
            return -1;
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';
        for (size_t i = 0; i < len; i++)
        {
            unsigned char ch = (unsigned char)buf[i];
            buf[i] = (char)toupper(ch);
        }
        return (int)len;
    }

    /* SDL interactive mode */
    memset(g_input_buf, 0, sizeof(g_input_buf));
    g_input_len = 0;
    g_input_cursor = 0;
    g_input_active = 1;
    g_input_prompt_line = g_cur_line;
    g_input_prompt_col = g_cur_col;

    /* Initialize browse cursor to the current line (bottom) */
    g_browse_line = (g_line_count > 0) ? (g_line_count - 1) : 0;

    /* Initialize scroll position: position viewport so prompt is visible at bottom
       This allows scrolling up to see history, and down to return to prompt */
    if (g_line_count > 0)
    {
        if (g_line_count >= g_rows)
        {
            /* Position viewport so prompt appears at the last visible row */
            g_view_start = g_line_count - g_rows;
        }
        else
        {
            /* Show from top if fewer lines than viewport height */
            g_view_start = 0;
        }
    }
    else
    {
        g_view_start = 0;
    }

    while (g_running && g_input_active)
    {
        /* Check for Ctrl-C (interrupt signal) */
        if (executor_check_interrupt())
        {
            g_input_active = 0;
            g_input_len = 0;
            return -1;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                g_running = 0;
                g_input_active = 0;
            }
            else if (event.type == SDL_TEXTINPUT && g_input_len < (int)sizeof(g_input_buf) - 1)
            {
                const char *text = event.text.text;
                for (int i = 0; text[i]; i++)
                {
                    unsigned char ch = (unsigned char)text[i];
                    if (ch >= 32 && ch < 127)
                    {
                        memmove(&g_input_buf[g_input_cursor + 1], &g_input_buf[g_input_cursor],
                                g_input_len - g_input_cursor);
                        g_input_buf[g_input_cursor] = (char)toupper(ch);
                        g_input_len++;
                        g_input_cursor++;
                    }
                }
                render_text();
            }
            else if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Keymod mod = event.key.keysym.mod;

                /* Ctrl+C interrupt */
                if (key == SDLK_c && (mod & KMOD_CTRL))
                {
                    executor_trigger_interrupt();
                    termio_write("^C\n");
                    g_input_active = 0;
                    g_input_len = 0;
                    return -1;
                }

                if (key == SDLK_b && (mod & KMOD_CTRL))
                {
                    g_show_browse_highlight = !g_show_browse_highlight;
                    render_text();
                }
                /* Handle Cmd+V (macOS) or Ctrl+V (other) for paste */
                else if (key == SDLK_v && ((mod & KMOD_GUI) || (mod & KMOD_CTRL)))
                {
                    char *clipboard = SDL_GetClipboardText();
                    if (clipboard && clipboard[0] != '\0')
                    {
                        /* Check if clipboard contains newlines (multi-line paste) */
                        int has_newlines = 0;
                        for (int i = 0; clipboard[i]; i++)
                        {
                            if (clipboard[i] == '\n' || clipboard[i] == '\r')
                            {
                                has_newlines = 1;
                                break;
                            }
                        }

                        if (has_newlines)
                        {
                            /* Multi-line paste: just insert first line, let user handle rest manually */
                            /* Or better: insert only printable chars from first line until newline */
                            for (int i = 0; clipboard[i] && g_input_len < (int)sizeof(g_input_buf) - 1; i++)
                            {
                                unsigned char ch = (unsigned char)clipboard[i];
                                if (ch == '\n' || ch == '\r')
                                    break; /* Stop at first newline */
                                if (ch >= 32 && ch < 127)
                                {
                                    memmove(&g_input_buf[g_input_cursor + 1], &g_input_buf[g_input_cursor],
                                            g_input_len - g_input_cursor);
                                    g_input_buf[g_input_cursor] = (char)toupper(ch);
                                    g_input_len++;
                                    g_input_cursor++;
                                }
                            }
                        }
                        else
                        {
                            /* Single line paste: insert all printable characters */
                            for (int i = 0; clipboard[i] && g_input_len < (int)sizeof(g_input_buf) - 1; i++)
                            {
                                unsigned char ch = (unsigned char)clipboard[i];
                                if (ch >= 32 && ch < 127)
                                {
                                    memmove(&g_input_buf[g_input_cursor + 1], &g_input_buf[g_input_cursor],
                                            g_input_len - g_input_cursor);
                                    g_input_buf[g_input_cursor] = (char)toupper(ch);
                                    g_input_len++;
                                    g_input_cursor++;
                                }
                            }
                        }
                        SDL_free(clipboard);
                        render_text();
                    }
                }
                else if (key == SDLK_RETURN)
                {
                    /* Check if browsing a previous line (not at current prompt) */
                    if (g_browse_line < g_line_count - 1)
                    {
                        /* Extract line text from scrollback */
                        int line_idx = (g_line_start + g_browse_line) % SCROLLBACK_MAX;
                        char line_text[256];
                        memset(line_text, 0, sizeof(line_text));

                        /* Copy line content, trimming trailing spaces */
                        int len = 0;
                        for (int col = 0; col < g_cols && len < (int)sizeof(line_text) - 1; col++)
                        {
                            if (g_lines[line_idx].cells[col] != ' ')
                                len = col + 1;
                        }
                        for (int col = 0; col < len; col++)
                        {
                            line_text[col] = g_lines[line_idx].cells[col];
                        }
                        line_text[len] = '\0';

                        /* Check if line starts with a line number */
                        if (line_text[0] != '\0' && isdigit((unsigned char)line_text[0]))
                        {
                            /* Parse line number */
                            int line_num = 0;
                            int i = 0;
                            while (i < len && isdigit((unsigned char)line_text[i]))
                            {
                                line_num = line_num * 10 + (line_text[i] - '0');
                                i++;
                            }

                            /* Return special command to invoke EDIT on this line number */
                            /* Format: "EDIT line_number" */
                            snprintf(buf, maxlen, "EDIT %d", line_num);
                            g_input_active = 0;
                            return strlen(buf);
                        }
                        else
                        {
                            /* Line doesn't have a line number, copy it as-is */
                            strncpy(buf, line_text, maxlen - 1);
                            buf[maxlen - 1] = '\0';
                            g_input_active = 0;
                            return strlen(buf);
                        }
                    }
                    else
                    {
                        /* At prompt line - normal return */
                        g_input_active = 0;
                    }
                }
                else if (key == SDLK_ESCAPE)
                {
                    /* ESC to cancel input (like Ctrl-C) */
                    g_input_buf[0] = '\0';
                    g_input_len = 0;
                    g_input_cursor = 0;
                    g_input_active = 0;
                    render_text();
                }
                else if (key == SDLK_BACKSPACE && g_input_cursor > 0)
                {
                    g_input_cursor--;
                    memmove(&g_input_buf[g_input_cursor], &g_input_buf[g_input_cursor + 1],
                            g_input_len - g_input_cursor - 1);
                    g_input_len--;
                    render_text();
                }
                else if (key == SDLK_DELETE && g_input_cursor < g_input_len)
                {
                    memmove(&g_input_buf[g_input_cursor], &g_input_buf[g_input_cursor + 1],
                            g_input_len - g_input_cursor - 1);
                    g_input_len--;
                    render_text();
                }
                else if (key == SDLK_LEFT && g_input_cursor > 0)
                {
                    g_input_cursor--;
                    render_text();
                }
                else if (key == SDLK_RIGHT && g_input_cursor < g_input_len)
                {
                    g_input_cursor++;
                    render_text();
                }
                else if (key == SDLK_HOME)
                {
                    g_input_cursor = 0;
                    render_text();
                }
                else if (key == SDLK_END)
                {
                    g_input_cursor = g_input_len;
                    render_text();
                }
                else if (key == SDLK_UP)
                {
                    /* Move cursor up through scrollback lines */
                    if (g_browse_line > 0)
                    {
                        g_browse_line--;
                        /* Adjust viewport if cursor moves above visible area */
                        if (g_browse_line < g_view_start)
                            g_view_start = g_browse_line;
                    }
                    render_text();
                }
                else if (key == SDLK_DOWN)
                {
                    /* Move cursor down through scrollback lines */
                    if (g_line_count > 0 && g_browse_line < g_line_count - 1)
                    {
                        g_browse_line++;
                        /* Adjust viewport if cursor moves below visible area */
                        int viewport_bottom = g_view_start + g_rows - 1;
                        if (g_browse_line > viewport_bottom)
                            g_view_start = g_browse_line - g_rows + 1;
                    }
                    render_text();
                }
                else if (key == SDLK_LEFT)
                {
                    /* Scroll left (when input cursor is already at start) */
                    if (g_col_start > 0)
                        g_col_start--;
                    render_text();
                }
                else if (key == SDLK_RIGHT)
                {
                    /* Scroll right (when input cursor is already at end) */
                    if (g_col_start < TERM_COLS - g_cols)
                        g_col_start++;
                    render_text();
                }
            }
        }
        /* Always render to show blinking cursor */
        render_text();
        SDL_Delay(10);
    }

    int len = g_input_len < maxlen ? g_input_len : maxlen - 1;
    memcpy(buf, g_input_buf, len);
    buf[len] = '\0';

    /* Echo input to scrollback buffer before newline */
    for (int i = 0; i < len; i++)
        termio_write_char(buf[i]);

    termio_write_char('\n');
    return g_running ? len : -1;
}

void termio_handle_events(void)
{
    if (!g_sdl_enabled)
        return;

    /* Check for Ctrl-C (interrupt signal) */
    executor_check_interrupt();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            g_running = 0;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            SDL_Keycode key = event.key.keysym.sym;
            SDL_Keymod mod = event.key.keysym.mod;

            /* Ctrl+C interrupt - just set flag, don't exit interpreter */
            if (key == SDLK_c && (mod & KMOD_CTRL))
            {
                executor_trigger_interrupt();
            }
            else if (key == SDLK_UP)
            {
                if (g_scroll_line > 0)
                {
                    g_scroll_line--;
                    if (g_scroll_line < g_view_start)
                        g_view_start = g_scroll_line;
                }
                render_text();
            }
            else if (key == SDLK_DOWN)
            {
                if (g_scroll_line < g_line_count - 1)
                {
                    g_scroll_line++;
                    int viewport_bottom = g_view_start + g_rows - 1;
                    if (g_scroll_line > viewport_bottom)
                        g_view_start = g_scroll_line - g_rows + 1;
                }
                render_text();
            }
            else if (key == SDLK_LEFT)
            {
                /* Scroll left */
                if (g_col_start > 0)
                    g_col_start--;
                render_text();
            }
            else if (key == SDLK_RIGHT)
            {
                /* Scroll right */
                if (g_col_start < TERM_COLS - g_cols)
                    g_col_start++;
                render_text();
            }
            else
            {
                /* Buffer regular key press for INKEY$ polling */
                if (key >= 32 && key < 127)
                {
                    g_polled_key = key;
                }
                else if (key == SDLK_RETURN)
                {
                    g_polled_key = '\n';
                }
            }
        }
    }
}

int termio_poll_key(void)
{
    /* Return buffered key press from termio_handle_events() */
    int key = g_polled_key;
    g_polled_key = -1; /* Clear buffer after reading */
    return key;
}

void termio_set_title(const char *title)
{
    if (g_sdl_enabled && g_window && title)
        SDL_SetWindowTitle(g_window, title);
}

void termio_render_graphics(void)
{
    if (!g_sdl_enabled || !g_renderer || !g_window || !video_graphics_active())
        return;
    int gw = video_memory.width * 2;
    int gh = video_memory.height * 2;
    SDL_SetWindowSize(g_window, gw, gh);
    render_video_memory();
}

#ifdef __APPLE__
/* Generate WAV with sine wave and play via afplay when SDL audio fails. */
static void beep_afplay_fallback(int duration_ms, int freq, int sample_rate, int num_samples)
{
    unsigned char wav_header[44];
    size_t data_size = (size_t)num_samples * 2;
    uint32_t file_size = (uint32_t)(36 + data_size);

    /* RIFF header */
    wav_header[0] = 'R';
    wav_header[1] = 'I';
    wav_header[2] = 'F';
    wav_header[3] = 'F';
    wav_header[4] = (unsigned char)(file_size);
    wav_header[5] = (unsigned char)(file_size >> 8);
    wav_header[6] = (unsigned char)(file_size >> 16);
    wav_header[7] = (unsigned char)(file_size >> 24);
    wav_header[8] = 'W';
    wav_header[9] = 'A';
    wav_header[10] = 'V';
    wav_header[11] = 'E';
    /* fmt chunk */
    wav_header[12] = 'f';
    wav_header[13] = 'm';
    wav_header[14] = 't';
    wav_header[15] = ' ';
    wav_header[16] = 16;
    wav_header[17] = 0;
    wav_header[18] = 0;
    wav_header[19] = 0;
    wav_header[20] = 1;
    wav_header[21] = 0; /* PCM */
    wav_header[22] = 1;
    wav_header[23] = 0; /* mono */
    wav_header[24] = (unsigned char)(sample_rate);
    wav_header[25] = (unsigned char)(sample_rate >> 8);
    wav_header[26] = (unsigned char)(sample_rate >> 16);
    wav_header[27] = (unsigned char)(sample_rate >> 24);
    wav_header[28] = (unsigned char)(sample_rate * 2);
    wav_header[29] = (unsigned char)((sample_rate * 2) >> 8);
    wav_header[30] = (unsigned char)((sample_rate * 2) >> 16);
    wav_header[31] = (unsigned char)((sample_rate * 2) >> 24);
    wav_header[32] = 2;
    wav_header[33] = 0; /* block align */
    wav_header[34] = 16;
    wav_header[35] = 0; /* bits per sample */
    /* data chunk */
    wav_header[36] = 'd';
    wav_header[37] = 'a';
    wav_header[38] = 't';
    wav_header[39] = 'a';
    wav_header[40] = (unsigned char)(data_size);
    wav_header[41] = (unsigned char)(data_size >> 8);
    wav_header[42] = (unsigned char)(data_size >> 16);
    wav_header[43] = (unsigned char)(data_size >> 24);

    char path[64];
    strcpy(path, "/tmp/beep_XXXXXX");
    int fd = mkstemp(path);
    if (fd < 0)
        return;

    if (write(fd, wav_header, 44) != 44)
    {
        close(fd);
        unlink(path);
        return;
    }

    const double amplitude = 16000.0;
    const double two_pi_f = 2.0 * 3.14159265358979323846 * (double)freq;

#define BUF_SAMPLES 4096
    int16_t buf[BUF_SAMPLES];
    int written = 0;
    while (written < num_samples)
    {
        int to_gen = num_samples - written;
        if (to_gen > BUF_SAMPLES)
            to_gen = BUF_SAMPLES;
        for (int i = 0; i < to_gen; i++)
        {
            double t = (double)(written + i) / (double)sample_rate;
            double s = sin(two_pi_f * t) * amplitude;
            buf[i] = (int16_t)s;
        }
        if (write(fd, buf, (size_t)to_gen * 2) != (ssize_t)(to_gen * 2))
        {
            close(fd);
            unlink(path);
            return;
        }
        written += to_gen;
    }
#undef BUF_SAMPLES

    close(fd);

    /* Short beeps (< 100ms): non-blocking for games. Longer: blocking for music. */
    char cmd[256];
    if (duration_ms < 100)
    {
        snprintf(cmd, sizeof(cmd), "(afplay %s 2>/dev/null; rm -f %s) &", path, path);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "afplay %s 2>/dev/null; rm -f %s", path, path);
    }
    (void)system(cmd);
}
#endif

/* Play a tone with SDL audio (duration + freq). Fallback to afplay on macOS if SDL fails. */
void termio_beep(int duration_ms, int freq_hz)
{
    if (duration_ms <= 0 || duration_ms > 5000)
        return;

    const int sample_rate = 44100;
    int freq = freq_hz;
    if (freq < 20)
        freq = 20;
    if (freq > 4000)
        freq = 4000;

    int num_samples = (int)((long)duration_ms * sample_rate / 1000);
    if (num_samples <= 0)
        return;

    SDL_AudioSpec want = {0};
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 4096;
    want.callback = NULL;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL,
                                                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (dev == 0)
    {
#ifdef __APPLE__
        /* SDL audio failed; generate WAV and play with afplay */
        beep_afplay_fallback(duration_ms, freq, sample_rate, num_samples);
#endif
        return;
    }

    Sint16 *samples = (Sint16 *)SDL_malloc((size_t)num_samples * sizeof(Sint16));
    if (!samples)
    {
        SDL_CloseAudioDevice(dev);
        return;
    }

    const double amplitude = 16000.0;
    const double two_pi_f = 2.0 * 3.14159265358979323846 * (double)freq;

    for (int i = 0; i < num_samples; i++)
    {
        double t = (double)i / (double)sample_rate;
        double s = sin(two_pi_f * t) * amplitude;
        samples[i] = (Sint16)s;
    }

    SDL_PauseAudioDevice(dev, 0);
    if (SDL_QueueAudio(dev, samples, (Uint32)(num_samples * sizeof(Sint16))) == 0)
    {
        Uint32 queued = SDL_GetQueuedAudioSize(dev);
        int wait_ms = 0;
        while (queued > 0 && wait_ms < duration_ms + 200)
        {
            SDL_Delay(10);
            wait_ms += 10;
            queued = SDL_GetQueuedAudioSize(dev);
        }
        SDL_Delay(20);
    }

    SDL_free(samples);
    SDL_CloseAudioDevice(dev);
}

/* Play a tone with harmonics: base_freq + harmonic overtones */
void termio_sound_harmonics(int base_freq, const int *harmonics, const double *intensities,
                            int num_harmonics, int duration_ms)
{
    if (duration_ms <= 0 || duration_ms > 5000)
        return;
    if (num_harmonics <= 0)
        return;
    if (base_freq < 20)
        base_freq = 20;
    if (base_freq > 4000)
        base_freq = 4000;

    const int sample_rate = 44100;
    int num_samples = (int)((long)duration_ms * sample_rate / 1000);
    if (num_samples <= 0)
        return;

    SDL_AudioSpec want = {0};
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 4096;
    want.callback = NULL;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL,
                                                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (dev == 0)
    {
#ifdef __APPLE__
        /* SDL audio failed; fall back to simple beep */
        termio_beep(duration_ms, base_freq);
#endif
        return;
    }

    Sint16 *samples = (Sint16 *)SDL_malloc((size_t)num_samples * sizeof(Sint16));
    if (!samples)
    {
        SDL_CloseAudioDevice(dev);
        return;
    }

    const double amplitude = 16000.0;
    const double two_pi = 2.0 * 3.14159265358979323846;

    /* Generate waveform as sum of harmonics */
    for (int i = 0; i < num_samples; i++)
    {
        double t = (double)i / (double)sample_rate;
        double sample_value = 0.0;

        /* Sum each harmonic */
        for (int h = 0; h < num_harmonics; h++)
        {
            int harmonic_num = harmonics[h];
            if (harmonic_num <= 0)
                continue;

            double intensity = intensities[h];
            if (intensity < 0.0)
                intensity = 0.0;
            if (intensity > 1.0)
                intensity = 1.0;

            int harmonic_freq = base_freq * harmonic_num;
            if (harmonic_freq > 20000)
                harmonic_freq = 20000;

            double harmonic_phase = two_pi * harmonic_freq * t;
            sample_value += sin(harmonic_phase) * intensity;
        }

        /* Normalize to prevent clipping */
        sample_value = sample_value / (double)num_harmonics;
        sample_value *= amplitude;
        samples[i] = (Sint16)sample_value;
    }

    SDL_PauseAudioDevice(dev, 0);
    if (SDL_QueueAudio(dev, samples, (Uint32)(num_samples * sizeof(Sint16))) == 0)
    {
        Uint32 queued = SDL_GetQueuedAudioSize(dev);
        int wait_ms = 0;
        while (queued > 0 && wait_ms < duration_ms + 200)
        {
            SDL_Delay(10);
            wait_ms += 10;
            queued = SDL_GetQueuedAudioSize(dev);
        }
        SDL_Delay(20);
    }

    SDL_free(samples);
    SDL_CloseAudioDevice(dev);
}

#endif /* USE_SDL */
