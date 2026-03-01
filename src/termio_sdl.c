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
static int g_cursor_row = 0;
static int g_cursor_col = 0;
static SDL_Color g_fg_color = {200, 200, 200, 255};
static SDL_Color g_bg_color = {0, 0, 0, 255};

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
        memset(g_screen[i], ' ', TERM_COLS);

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
        memset(g_screen[i], ' ', TERM_COLS);

    g_cursor_row = 0;
    g_cursor_col = 0;
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
            /* Scroll up */
            for (int i = 0; i < TERM_ROWS - 1; i++)
                memcpy(g_screen[i], g_screen[i + 1], TERM_COLS);
            memset(g_screen[TERM_ROWS - 1], ' ', TERM_COLS);
            g_cursor_row = TERM_ROWS - 1;
        }
    }
    else if (c == '\r')
    {
        g_cursor_col = 0;
    }
    else
    {
        if (g_cursor_col >= TERM_COLS)
        {
            g_cursor_row++;
            g_cursor_col = 0;

            if (g_cursor_row >= TERM_ROWS)
            {
                for (int i = 0; i < TERM_ROWS - 1; i++)
                    memcpy(g_screen[i], g_screen[i + 1], TERM_COLS);
                memset(g_screen[TERM_ROWS - 1], ' ', TERM_COLS);
                g_cursor_row = TERM_ROWS - 1;
            }
        }

        g_screen[g_cursor_row][g_cursor_col] = c;
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

    for (int row = 0; row < TERM_ROWS; row++)
    {
        for (int col = 0; col < TERM_COLS; col++)
        {
            char c = g_screen[row][col];
            if (c == ' ')
                continue;

            char text[2] = {c, 0};
            SDL_Surface *surf = TTF_RenderText_Blended(g_font, text, g_fg_color);
            if (!surf)
                continue;

            SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderer, surf);
            /* Render at native pixel coordinates for HiDPI sharpness */
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

    SDL_RenderPresent(g_renderer);
}

void termio_set_colors(int fg, int bg)
{
    if (fg == 1 && bg == 0)
    {
        /* WOB: white on black */
        g_fg_color = (SDL_Color){200, 200, 200, 255};
        g_bg_color = (SDL_Color){0, 0, 0, 255};
    }
    else if (fg == 0 && bg == 1)
    {
        /* BOW: black on white */
        g_fg_color = (SDL_Color){20, 20, 20, 255};
        g_bg_color = (SDL_Color){220, 220, 220, 255};
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
    if (!g_sdl_enabled)
        return;

    /* Clear screen */
    termio_clear();

    /* Center text vertically and horizontally */
    int start_row = 8;
    int lines[4];
    lines[0] = start_row;     /* Name */
    lines[1] = start_row + 2; /* Version */
    lines[2] = start_row + 4; /* Copyright */
    lines[3] = start_row + 7; /* Prompt */

    /* Draw welcome text */
    char line_text[TERM_COLS + 1];

    /* Name centered */
    snprintf(line_text, sizeof(line_text), "%*s", (TERM_COLS + (int)strlen(name)) / 2, name);
    for (int i = 0; i < TERM_COLS && line_text[i]; i++)
        g_screen[lines[0]][i] = line_text[i];

    /* Version centered */
    snprintf(line_text, sizeof(line_text), "%*s", (TERM_COLS + (int)strlen(version)) / 2, version);
    for (int i = 0; i < TERM_COLS && line_text[i]; i++)
        g_screen[lines[1]][i] = line_text[i];

    /* Copyright centered */
    const char *copyright = "2026. Meltingcaps.com";
    snprintf(line_text, sizeof(line_text), "%*s", (TERM_COLS + (int)strlen(copyright)) / 2, copyright);
    for (int i = 0; i < TERM_COLS && line_text[i]; i++)
        g_screen[lines[2]][i] = line_text[i];

    /* Prompt */
    const char *prompt = "[Press Ctrl-C to exit]";
    snprintf(line_text, sizeof(line_text), "%*s", (TERM_COLS + (int)strlen(prompt)) / 2, prompt);
    for (int i = 0; i < TERM_COLS && line_text[i]; i++)
        g_screen[lines[3]][i] = line_text[i];

    /* Display and wait for Ctrl-C */
    int done = 0;
    int blink_state = 0;
    Uint32 last_blink = SDL_GetTicks();

    while (!done)
    {
        termio_present();

        SDL_Event event;
        if (SDL_WaitEventTimeout(&event, 100))
        {
            if (event.type == SDL_QUIT)
                exit(0);
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_c &&
                    (event.key.keysym.mod & KMOD_CTRL))
                {
                    done = 1;
                }
                else
                {
                    /* Any other key also dismisses */
                    done = 1;
                }
            }
        }

        /* Blink effect for waiting */
        Uint32 now = SDL_GetTicks();
        if (now - last_blink > 500)
        {
            blink_state = !blink_state;
            last_blink = now;
        }
    }

    /* Clear screen and show Ok prompt */
    termio_clear();
    g_cursor_row = 0;
    g_cursor_col = 0;
}