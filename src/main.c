#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "runtime.h"
#include "executor.h"
#include "symtable.h"
#include "compat.h"
#include "termio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

typedef struct
{
    const char *name;
    const char *version;
    const char *build_datetime;
} VersionInfo;

static const VersionInfo g_version_info = {
    "Basic++ Interpreter",
    "Version 0.3.2",
    __DATE__ " " __TIME__};

typedef struct
{
    char *text;
} StoredLine;

static volatile sig_atomic_t g_interrupt = 0;
static char g_loaded_program_dir[PATH_MAX] = ""; /* Directory of loaded BASIC program */

static void handle_sigint(int sig)
{
    (void)sig;
    g_interrupt = 1;
}

static void append_line(StoredLine **lines, int *count, int *cap, const char *text)
{
    if (*count >= *cap)
    {
        *cap = (*cap == 0) ? 32 : (*cap * 2);
        *lines = xrealloc(*lines, (size_t)(*cap) * sizeof(StoredLine));
    }
    (*lines)[*count].text = xstrdup(text);
    (*count)++;
}

static void clear_program(StoredLine **lines, int *count, int *cap)
{
    for (int i = 0; i < *count; i++)
    {
        free((*lines)[i].text);
    }
    free(*lines);
    *lines = NULL;
    *count = 0;
    *cap = 0;
}

static void list_program(StoredLine *lines, int count)
{
    /* Calculate width needed to right-align the largest line number */
    int width = 1;
    int tmp = count;
    while (tmp >= 10)
    {
        width++;
        tmp /= 10;
    }

    for (int i = 0; i < count; i++)
    {
        const char *text = lines[i].text ? lines[i].text : "";

        /* Strip leading old-style BASIC line number if present */
        const char *display_text = text;
        if (isdigit((unsigned char)*text))
        {
            char *after;
            strtol(text, &after, 10);
            if (after != text && isspace((unsigned char)*after))
                display_text = after + 1;
        }

        /* Write sequential line number (right-aligned) in grey, then two spaces */
        char num_buf[16];
        snprintf(num_buf, sizeof(num_buf), "%*d  ", width, i + 1);
        termio_set_write_color(COL_COMMENT);
        termio_write(num_buf);

        /* Write the code with syntax highlighting */
        termio_write_highlighted(display_text);
    }
}

static char *build_program_text(StoredLine *lines, int count)
{
    size_t total = 0;
    for (int i = 0; i < count; i++)
        total += (lines[i].text ? strlen(lines[i].text) : 0) + 1;

    char *buf = xmalloc(total + 1);
    buf[0] = '\0';
    for (int i = 0; i < count; i++)
    {
        if (lines[i].text)
            strcat(buf, lines[i].text);
        strcat(buf, "\n");
    }
    return buf;
}

static char *parse_filename_arg(const char *line)
{
    const char *p = line;
    while (*p && !isspace((unsigned char)*p))
        p++;
    while (*p && isspace((unsigned char)*p))
        p++;

    if (*p == '"')
    {
        p++;
        const char *start = p;
        while (*p && *p != '"')
            p++;
        size_t len = (size_t)(p - start);
        char *out = xmalloc(len + 1);
        memcpy(out, start, len);
        out[len] = '\0';
        return out;
    }

    if (*p == '\0')
        return NULL;

    const char *start = p;
    while (*p && !isspace((unsigned char)*p))
        p++;
    size_t len = (size_t)(p - start);
    char *out = xmalloc(len + 1);
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int load_program_file(StoredLine **lines, int *count, int *cap, const char *filename)
{
    /* Resolve file path:
     * 1. If absolute path, use it directly
     * 2. If relative and BASIC_CWD is set, prepend BASIC_CWD
     * 3. If relative and BASIC_CWD not set, use as-is (current directory)
     */
    char resolved_path[PATH_MAX];
    int is_absolute = filename[0] == '/';

    if (is_absolute)
    {
        /* Absolute path - use directly */
        strncpy(resolved_path, filename, PATH_MAX - 1);
        resolved_path[PATH_MAX - 1] = '\0';
    }
    else
    {
        /* Relative path - check BASIC_CWD */
        const char *basic_cwd = getenv("BASIC_CWD");
        if (basic_cwd && *basic_cwd)
        {
            /* Prepend BASIC_CWD */
            snprintf(resolved_path, PATH_MAX, "%s/%s", basic_cwd, filename);
        }
        else
        {
            /* Use as-is (current directory) */
            strncpy(resolved_path, filename, PATH_MAX - 1);
            resolved_path[PATH_MAX - 1] = '\0';
        }
    }

    FILE *fp = fopen(resolved_path, "r");
    if (!fp)
    {
        termio_printf("?LOAD ERROR: %s\n", strerror(errno));
        return 1;
    }

    clear_program(lines, count, cap);

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
        {
            buffer[--len] = '\0';
        }

        /* Use trimmed pointer only for blank/comment/line-number detection */
        char *trimmed = buffer;
        while (*trimmed && isspace((unsigned char)*trimmed))
            trimmed++;

        if (*trimmed == '\0' || *trimmed == '!')
            continue;

        /* Strip leading line number if present (backward compatibility) */
        if (isdigit((unsigned char)*trimmed))
        {
            char *after;
            strtol(trimmed, &after, 10);
            if (after != trimmed && isspace((unsigned char)*after))
            {
                /* Old numbered line: skip number + one space, preserve any further indentation */
                after++;
                append_line(lines, count, cap, after);
                continue;
            }
        }

        /* No line number: store buffer as-is, preserving original indentation */
        append_line(lines, count, cap, buffer);
    }

    fclose(fp);

    /* Extract directory from resolved_path and store it */
    const char *last_slash = strrchr(resolved_path, '/');
    if (last_slash)
    {
        int dir_len = (int)(last_slash - resolved_path);
        if (dir_len > PATH_MAX - 1)
            dir_len = PATH_MAX - 1;
        strncpy(g_loaded_program_dir, resolved_path, (size_t)dir_len);
        g_loaded_program_dir[dir_len] = '\0';
    }
    else
    {
        /* No directory component, use current directory */
        strncpy(g_loaded_program_dir, ".", PATH_MAX - 1);
        g_loaded_program_dir[PATH_MAX - 1] = '\0';
    }

    return 0;
}

static int save_program_file(StoredLine *lines, int count, const char *filename)
{
    /* Resolve file path:
     * 1. If absolute path, use it directly
     * 2. If relative and BASIC_CWD is set, prepend BASIC_CWD
     * 3. If relative and BASIC_CWD not set, use as-is (current directory)
     */
    char resolved_path[PATH_MAX];
    int is_absolute = filename[0] == '/';

    if (is_absolute)
    {
        /* Absolute path - use directly */
        strncpy(resolved_path, filename, PATH_MAX - 1);
        resolved_path[PATH_MAX - 1] = '\0';
    }
    else
    {
        /* Relative path - check BASIC_CWD */
        const char *basic_cwd = getenv("BASIC_CWD");
        if (basic_cwd && *basic_cwd)
        {
            /* Prepend BASIC_CWD */
            snprintf(resolved_path, PATH_MAX, "%s/%s", basic_cwd, filename);
        }
        else
        {
            /* Use as-is (current directory) */
            strncpy(resolved_path, filename, PATH_MAX - 1);
            resolved_path[PATH_MAX - 1] = '\0';
        }
    }

    FILE *fp = fopen(resolved_path, "w");
    if (!fp)
    {
        termio_printf("?SAVE ERROR: Cannot open file '%s' for writing\n", resolved_path);
        termio_printf("?SAVE ERROR: %s\n", strerror(errno));
        return 1;
    }

    for (int i = 0; i < count; i++)
    {
        if (fprintf(fp, "%s\n", lines[i].text ? lines[i].text : "") < 0)
        {
            termio_write("?SAVE ERROR: Write failed\n");
            fclose(fp);
            return 1;
        }
    }

    if (fclose(fp) != 0)
    {
        termio_write("?SAVE ERROR: Failed to close file\n");
        return 1;
    }

    termio_printf("FILE SAVED to %s\n", filename);
    return 0;
}

/* Global save context for SAVE statement in programs */
static StoredLine *g_save_lines = NULL;
static int g_save_line_count = 0;
static int g_save_lines_cap = 0;

/* Global runtime reference for MERGE callback */
static RuntimeState *g_runtime = NULL;

static int save_callback(const char *filename)
{
    return save_program_file(g_save_lines, g_save_line_count, filename);
}

/* Global delete context for DELETE statement in programs */
static int delete_callback(int start_line, int end_line)
{
    /* Line-number based DELETE no longer supported */
    (void)start_line;
    (void)end_line;
    return 0;
}

static int merge_callback(const char *filename)
{
    /* Line-number based MERGE no longer supported */
    (void)filename;
    return 0;
}

/* Forward declarations */
static int run_program_text_from_line(RuntimeState *runtime, const char *program_text, int start_line_num);

static int run_program_text(RuntimeState *runtime, const char *program_text)
{
    return run_program_text_from_line(runtime, program_text, -1);
}

static int run_program_text_from_line(RuntimeState *runtime, const char *program_text, int start_line_num)
{
    Lexer *lexer = lexer_create(program_text);
    Token *tokens = lexer_tokenize(lexer);
    Parser *parser = parser_create(tokens, lexer_token_count(lexer));
    Program *program = parse_program(parser);

    if (parser_has_error(parser))
    {
        termio_printf("Parse error: %s\n", parser_error_message(parser));
        parser_free(parser);
        lexer_free(lexer);
        ast_program_free(program);
        return 1;
    }

    SymbolTable *symtable = symtable_create();
    if (symtable_analyze_program(symtable, program) != 0)
    {
        termio_printf("Symbol table analysis failed\n");
        symtable_free(symtable);
        parser_free(parser);
        lexer_free(lexer);
        ast_program_free(program);
        return 1;
    }

    int result;
    if (start_line_num > 0)
    {
        result = execute_program_from_line(runtime, program, start_line_num);
    }
    else
    {
        result = execute_program(runtime, program);
    }

    symtable_free(symtable);
    parser_free(parser);
    lexer_free(lexer);
    ast_program_free(program);
    return result;
}

static int starts_with_keyword(const char *line, const char *keyword)
{
    size_t len = strlen(keyword);
    if (strncasecmp(line, keyword, len) != 0)
        return 0;
    if (line[len] == '\0' || isspace((unsigned char)line[len]))
        return 1;
    return 0;
}

static void run_interactive(void)
{
    StoredLine *lines = NULL;
    int line_count = 0;
    int line_cap = 0;

    if (termio_init(80, 24, 1) < 0)
    {
        /* SDL2 failed, fall back to stdio mode */
        printf("(c) 1978 Tandy Corporation\n");
        fflush(stdout);
    }
    else
    {
        /* SDL2 initialized, show welcome screen */
        termio_show_welcome(g_version_info.name, g_version_info.version);
    }
    termio_set_title("Basic++");

    RuntimeState *runtime = runtime_create();
    runtime_set_memory_size(runtime, 32768); /* Default memory size */
    executor_set_interrupt_flag(&g_interrupt);

    signal(SIGINT, handle_sigint);

    char input[1024];
    while (1)
    {
        /* Check for interrupt at start of loop */
        if (g_interrupt)
        {
            g_interrupt = 0;
            /* Always move to new line before BREAK (PRINT@ may have moved cursor) */
            /*termio_write("\n"); */
            termio_write("BREAK\n");
            runtime_set_output_col(runtime, 0);
            runtime_set_output_pending(runtime, 0);
            continue;
        }

        if (runtime_get_output_pending(runtime))
        {
            termio_write("\n");
            runtime_set_output_pending(runtime, 0);
        }

        (void)runtime; /* suppress unused after potential future refactor */
        termio_write("Ok ");

        termio_handle_events(); /* Handle any pending events (scrolling, etc.) */

        int len = termio_readline(input, sizeof(input));

        /* Check for interrupt after reading input */
        if (g_interrupt)
        {
            g_interrupt = 0;
            /* Always move to new line before BREAK (PRINT@ may have moved cursor) */
            termio_write("\n");
            termio_write("BREAK\n");
            runtime_set_output_col(runtime, 0);
            runtime_set_output_pending(runtime, 0);
            continue;
        }

        if (len == -2)
        {
            /* Ctrl-C in SDL readline */
            termio_write("BREAK\n");
            runtime_set_output_col(runtime, 0);
            runtime_set_output_pending(runtime, 0);
            continue;
        }

        if (len < 0)
        {
            termio_write("\n");
            break;
        }

        char *p = input;
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0')
            continue;

        if (*p == '!')
        {
            p++;
            while (*p && isspace((unsigned char)*p))
                p++;
            if (*p != '\0')
            {
                /* Execute shell command and capture output */
                termio_write("\n");
                FILE *fp = popen(p, "r");
                if (fp == NULL)
                {
                    termio_write("?SHELL ERROR\n");
                }
                else
                {
                    char output_line[1024];
                    while (fgets(output_line, sizeof(output_line), fp) != NULL)
                    {
                        termio_write(output_line);
                    }
                    int status = pclose(fp);
                    if (status != 0)
                    {
                        termio_printf("?EXIT CODE: %d\n", WEXITSTATUS(status));
                    }
                }
                termio_write("\n");
            }
            else
            {
                termio_write("?MISSING COMMAND\n");
            }
            continue;
        }

        if (starts_with_keyword(p, "CLS"))
        {
            termio_clear();
            continue;
        }

        if (strcmp(p, "WOB") == 0 || strcasecmp(p, "WOB") == 0)
        {
            termio_set_colors(1, 0); /* white on black */
            continue;
        }

        if (strcmp(p, "BOW") == 0 || strcasecmp(p, "BOW") == 0)
        {
            termio_set_colors(0, 1); /* black on white */
            continue;
        }

        if (starts_with_keyword(p, "SYSTEM") || starts_with_keyword(p, "EXIT") || starts_with_keyword(p, "QUIT"))
        {
            break;
        }

        if (starts_with_keyword(p, "LIST"))
        {
            list_program(lines, line_count);
            continue;
        }

        if (starts_with_keyword(p, "VERSION"))
        {
            termio_printf("NAME: %s\n", g_version_info.name);
            termio_printf("VERSION: %s\n", g_version_info.version);
            termio_printf("BUILD: %s\n", g_version_info.build_datetime);
            continue;
        }

        if (starts_with_keyword(p, "EDIT"))
        {
            termio_write("?EDIT not available - use LOAD/SAVE with your editor\n");
            continue;
        }

        if (starts_with_keyword(p, "LOAD"))
        {
            char *fname = parse_filename_arg(p);
            if (!fname)
            {
                termio_write("?MISSING FILENAME\n");
            }
            else
            {
                int ret = load_program_file(&lines, &line_count, &line_cap, fname);
                if (ret == 0)
                {
                    termio_write("FILE LOADED\n");
                }
                free(fname);
            }
            continue;
        }

        if (starts_with_keyword(p, "SAVE"))
        {
            char *fname = parse_filename_arg(p);
            if (!fname)
            {
                termio_write("?SYNTAX: SAVE \"filename\"\n");
            }
            else
            {
                int ret = save_program_file(lines, line_count, fname);
                if (ret != 0)
                {
                    /* Error message already printed by save_program_file */
                }
                free(fname);
            }
            continue;
        }

        if (starts_with_keyword(p, "NEW"))
        {
            clear_program(&lines, &line_count, &line_cap);
            continue;
        }

        if (starts_with_keyword(p, "CLEAR"))
        {
            runtime_free(runtime);
            runtime = runtime_create();
            runtime_set_memory_size(runtime, 32768); /* Default memory size */
            g_interrupt = 0;
            continue;
        }

        if (starts_with_keyword(p, "RUN"))
        {
            const char *rest = p + 3;
            while (*rest == ' ' || *rest == '\t')
                rest++;

            if (starts_with_keyword(rest, "CHECK"))
            {
                char *program_text = build_program_text(lines, line_count);
                compat_clear_violations(g_compat_state);

                Lexer *lexer = lexer_create(program_text);
                Token *tokens = lexer_tokenize(lexer);
                Parser *parser = parser_create(tokens, lexer_token_count(lexer));
                Program *program = parse_program(parser);

                if (program)
                    compat_check_program_arrays(program, g_compat_state);

                compat_print_violations(g_compat_state);

                if (program)
                    ast_program_free(program);
                parser_free(parser);
                lexer_free(lexer);
                free(program_text);
                continue;
            }

            /* Parse optional line number argument - no longer supported */
            int start_line_num = -1;
            (void)start_line_num;

            char *program_text = build_program_text(lines, line_count);
            runtime_free(runtime);
            runtime = runtime_create();
            g_interrupt = 0;

            compat_clear_violations(g_compat_state);

            /* Set up SAVE and DELETE callbacks with current program lines */
            g_save_lines = lines;
            g_save_line_count = line_count;
            g_save_lines_cap = line_cap;
            g_runtime = runtime;
            runtime_set_save_callback(runtime, save_callback);
            runtime_set_delete_callback(runtime, delete_callback);
            runtime_set_merge_callback(runtime, merge_callback);

            /* Change to loaded program directory for file I/O */
            char saved_cwd[PATH_MAX];
            if (getcwd(saved_cwd, sizeof(saved_cwd)) == NULL)
            {
                strcpy(saved_cwd, "."); /* Fallback */
            }

            if (g_loaded_program_dir[0] != '\0' && chdir(g_loaded_program_dir) == 0)
            {
                run_program_text(runtime, program_text);
                chdir(saved_cwd); /* Restore original directory */
            }
            else
            {
                run_program_text(runtime, program_text);
            }

            free(program_text);

            /* After program ends, move to new line (PRINT@ may have moved cursor) */
            termio_write("\n");
            runtime_set_output_col(runtime, 0);
            runtime_set_output_pending(runtime, 0);
            continue;
        }

        if (strncasecmp(p, "AUTO", 4) == 0)
        {
            /* AUTO mode removed - line numbers are no longer supported */
            termio_write("?AUTO mode no longer available\n");
            continue;
        }

        {
            char *temp = xmalloc(strlen(p) + 2);
            sprintf(temp, "%s\n", p);
            g_interrupt = 0;
            run_program_text(runtime, temp);
            free(temp);
        }
    }

    clear_program(&lines, &line_count, &line_cap);
    termio_shutdown();
}

int main(int argc, char *argv[])
{
    int strict_mode = 0;
    int dump_tokens = 0;
    const char *filename = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--strict") == 0)
        {
            strict_mode = 1;
        }
        else if (strcmp(argv[i], "--dump-tokens") == 0)
        {
            dump_tokens = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("TRS-80 BASIC Interpreter - AST Implementation\n\n");
            printf("Usage: %s [options] [filename]\n\n", argv[0]);
            printf("Options:\n");
            printf("  --strict        Enforce TRS-80 Level II BASIC compatibility\n");
            printf("  --dump-tokens   Print token stream and exit\n");
            printf("  --help, -h      Show this help message\n\n");
            printf("Interactive commands:\n");
            printf("  NEW         Clear program\n");
            printf("  VERSION     Show version information\n");
            printf("  LIST        Display program\n");
            printf("  RUN         Execute program\n");
            printf("  RUN CHECK   Check TRS-80 compatibility\n");
            printf("  LOAD \"file\" Load program from file\n");
            printf("  SAVE \"file\" Save program to file\n");
            printf("  SYSTEM      Exit interpreter\n\n");
            printf("Environment variables:\n");
            printf("  BASIC_CWD   Working directory for relative file paths\n\n");
            return 0;
        }
        else if (argv[i][0] != '-')
        {
            filename = argv[i];
        }
    }

    g_compat_state = compat_init(strict_mode);

    if (filename == NULL)
    {
        run_interactive();
        compat_free(g_compat_state);
        g_compat_state = NULL;
        return 0;
    }

    if (strict_mode)
    {
        printf("TRS-80 BASIC Interpreter - Strict Mode\n");
        printf("Only TRS-80 Level II BASIC syntax accepted.\n\n");
    }

    /* Resolve file path:
     * 1. If absolute path, use it directly
     * 2. If relative and BASIC_CWD is set, prepend BASIC_CWD
     * 3. If relative and BASIC_CWD not set, use as-is (current directory)
     */
    char file_to_run[PATH_MAX];
    int is_absolute = filename[0] == '/';

    if (is_absolute)
    {
        /* Absolute path - use directly */
        strncpy(file_to_run, filename, PATH_MAX - 1);
        file_to_run[PATH_MAX - 1] = '\0';
    }
    else
    {
        /* Relative path - check BASIC_CWD */
        const char *basic_cwd = getenv("BASIC_CWD");
        if (basic_cwd && *basic_cwd)
        {
            /* Prepend BASIC_CWD */
            snprintf(file_to_run, PATH_MAX, "%s/%s", basic_cwd, filename);
        }
        else
        {
            /* Use as-is (current directory) */
            strncpy(file_to_run, filename, PATH_MAX - 1);
            file_to_run[PATH_MAX - 1] = '\0';
        }
    }

    /* Load file through StoredLine system so line numbers are auto-assigned
     * consistently with the REPL path */
    StoredLine *lines = NULL;
    int line_count = 0;
    int line_cap = 0;
    if (load_program_file(&lines, &line_count, &line_cap, file_to_run) != 0)
    {
        compat_free(g_compat_state);
        g_compat_state = NULL;
        return 1;
    }

    char *program_text = build_program_text(lines, line_count);

    Lexer *lexer = lexer_create(program_text);
    Token *tokens = lexer_tokenize(lexer);

    if (dump_tokens)
    {
        int num_tokens = lexer_token_count(lexer);
        for (int i = 0; i < num_tokens; i++)
        {
            printf("Token: %-12s Value: %-12s Line: %d\n",
                   token_type_name(tokens[i].type),
                   tokens[i].value ? tokens[i].value : "",
                   tokens[i].line_number);
        }
        lexer_free(lexer);
        free(program_text);
        clear_program(&lines, &line_count, &line_cap);
        return 0;
    }

    Parser *parser = parser_create(tokens, lexer_token_count(lexer));
    Program *program = parse_program(parser);

    if (parser_has_error(parser))
    {
        termio_printf("Parse error: %s\n", parser_error_message(parser));
        parser_free(parser);
        lexer_free(lexer);
        ast_program_free(program);
        free(program_text);
        clear_program(&lines, &line_count, &line_cap);
        return 1;
    }

    RuntimeState *runtime = runtime_create();
    g_save_lines = lines;
    g_save_line_count = line_count;
    g_save_lines_cap = line_cap;
    g_runtime = runtime;
    runtime_set_save_callback(runtime, save_callback);
    runtime_set_delete_callback(runtime, delete_callback);
    runtime_set_merge_callback(runtime, merge_callback);

    executor_set_interrupt_flag(&g_interrupt);
    signal(SIGINT, handle_sigint);

    SymbolTable *symtable = symtable_create();
    if (symtable_analyze_program(symtable, program) != 0)
    {
        termio_printf("Symbol table analysis failed\n");
        symtable_free(symtable);
        runtime_free(runtime);
        parser_free(parser);
        lexer_free(lexer);
        ast_program_free(program);
        free(program_text);
        return 1;
    }

    /* Change to program directory for file I/O (batch mode) */
    char saved_cwd[PATH_MAX];
    if (getcwd(saved_cwd, sizeof(saved_cwd)) == NULL)
    {
        strcpy(saved_cwd, "."); /* Fallback */
    }

    int result;
    if (g_loaded_program_dir[0] != '\0' && chdir(g_loaded_program_dir) == 0)
    {
        result = execute_program(runtime, program);
        chdir(saved_cwd); /* Restore original directory */
    }
    else
    {
        result = execute_program(runtime, program);
    }

    clear_program(&lines, &line_count, &line_cap);
    symtable_free(symtable);
    runtime_free(runtime);
    parser_free(parser);
    lexer_free(lexer);
    ast_program_free(program);
    free(program_text);
    compat_free(g_compat_state);
    g_compat_state = NULL;

    return result;
}
