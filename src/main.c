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
    "Version 0.2.0",
    __DATE__ " " __TIME__};

typedef struct
{
    int line_number;
    char *text;
} StoredLine;

static volatile sig_atomic_t g_interrupt = 0;
static char g_loaded_program_dir[PATH_MAX] = ""; /* Directory of loaded BASIC program */

static void handle_sigint(int sig)
{
    (void)sig;
    g_interrupt = 1;
}

static int find_line_index(StoredLine *lines, int count, int line_number)
{
    int lo = 0, hi = count;
    while (lo < hi)
    {
        int mid = (lo + hi) / 2;
        if (lines[mid].line_number < line_number)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo < count && lines[lo].line_number == line_number)
        return lo;
    return -lo - 1;
}

static void insert_line(StoredLine **lines, int *count, int *cap, int line_number, const char *text)
{
    if (text == NULL || *text == '\0')
    {
        int idx = find_line_index(*lines, *count, line_number);
        if (idx >= 0)
        {
            free((*lines)[idx].text);
            memmove(&(*lines)[idx], &(*lines)[idx + 1], (size_t)(*count - idx - 1) * sizeof(StoredLine));
            (*count)--;
        }
        return;
    }

    int idx = find_line_index(*lines, *count, line_number);
    if (idx >= 0)
    {
        free((*lines)[idx].text);
        (*lines)[idx].text = xstrdup(text);
        return;
    }

    idx = -idx - 1;
    if (*count >= *cap)
    {
        *cap = (*cap == 0) ? 32 : (*cap * 2);
        *lines = xrealloc(*lines, (size_t)(*cap) * sizeof(StoredLine));
    }

    memmove(&(*lines)[idx + 1], &(*lines)[idx], (size_t)(*count - idx) * sizeof(StoredLine));
    (*lines)[idx].line_number = line_number;
    (*lines)[idx].text = xstrdup(text);
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

/* Check if word at p matches keyword (case-insensitive). p points to start of word. */
static int match_keyword(const char *p, const char *end, const char *kw)
{
    size_t len = strlen(kw);
    if ((size_t)(end - p) < len)
        return 0;
    if (strncasecmp(p, kw, len) != 0)
        return 0;
    if ((size_t)(end - p) > len && (isalnum((unsigned char)p[len]) || p[len] == '$'))
        return 0;
    return 1;
}

/* Replace line number refs (GOTO, GOSUB, THEN, RESTORE, ON...GOTO/GOSUB) using map. */
static char *replace_all_line_refs(const char *text, const int *map, int max_old)
{
    size_t text_len = strlen(text);
    size_t out_cap = text_len + 128;
    char *out = xmalloc(out_cap);
    size_t out_pos = 0;
    int in_line_ref_context = 0; /* 1=expect line num after GOTO/GOSUB/THEN/RESTORE, 2=in ON..GOTO list */

    for (size_t i = 0; i < text_len;)
    {
        if (isdigit((unsigned char)text[i]))
        {
            size_t j = i;
            int val = 0;
            while (j < text_len && isdigit((unsigned char)text[j]))
            {
                val = val * 10 + (text[j] - '0');
                j++;
            }
            int new_val = val;
            if (in_line_ref_context && val <= max_old && map[val] >= 0)
                new_val = map[val];
            if (in_line_ref_context)
                in_line_ref_context = (in_line_ref_context == 2) ? 2 : 0; /* Stay in 2 for next num */
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", new_val);
            size_t buf_len = strlen(buf);
            if (out_pos + buf_len >= out_cap)
            {
                out_cap *= 2;
                out = xrealloc(out, out_cap);
            }
            memcpy(out + out_pos, buf, buf_len + 1);
            out_pos += buf_len;
            i = j;
        }
        else if (isalpha((unsigned char)text[i]) || text[i] == '_')
        {
            size_t start = i;
            while (i < text_len && (isalnum((unsigned char)text[i]) || text[i] == '$' || text[i] == '_'))
                i++;
            const char *word_end = text + i;
            if (match_keyword(text + start, word_end, "GOTO") ||
                match_keyword(text + start, word_end, "GOSUB"))
            {
                in_line_ref_context = 2; /* Single or comma-separated list */
            }
            else if (match_keyword(text + start, word_end, "THEN") ||
                     match_keyword(text + start, word_end, "RESTORE"))
            {
                in_line_ref_context = 1; /* Single line number */
            }
            while (out_pos + (i - start) >= out_cap)
            {
                out_cap *= 2;
                out = xrealloc(out, out_cap);
            }
            memcpy(out + out_pos, text + start, i - start);
            out_pos += i - start;
        }
        else
        {
            if (text[i] != ',' && text[i] != ' ' && text[i] != '\t' && in_line_ref_context == 1)
                in_line_ref_context = 0;
            if (out_pos + 1 >= out_cap)
            {
                out_cap *= 2;
                out = xrealloc(out, out_cap);
            }
            out[out_pos++] = text[i++];
        }
    }
    out[out_pos] = '\0';
    return out;
}

/* RENUM [start] [, increment]. Defaults: start=10, increment=10. */
static int do_renum(StoredLine **lines, int *count, int *cap, int start, int increment)
{
    (void)cap;
    if (*count == 0)
    {
        termio_write("?NO PROGRAM\n");
        return -1;
    }
    if (start < 1 || increment < 1)
    {
        termio_write("?ILLEGAL FUNCTION CALL\n");
        return -1;
    }

    /* Build old_line -> new_line mapping (by order in lines array) */
    int *old_to_new = xmalloc((size_t)(*count) * sizeof(int));
    int max_old = 0;
    for (int i = 0; i < *count; i++)
    {
        int old = (*lines)[i].line_number;
        if (old > max_old)
            max_old = old;
        int new_num = start + i * increment;
        if (new_num > 65535)
        {
            free(old_to_new);
            termio_write("?OVERFLOW\n");
            return -1;
        }
        old_to_new[i] = new_num;
    }

    int *map = xmalloc((size_t)(max_old + 1) * sizeof(int));
    for (int i = 0; i <= max_old; i++)
        map[i] = -1;
    for (int i = 0; i < *count; i++)
        map[(*lines)[i].line_number] = old_to_new[i];

    /* Replace line refs in each line's text */
    for (int i = 0; i < *count; i++)
    {
        char *old_text = (*lines)[i].text;
        if (!old_text)
            continue;
        char *new_text = replace_all_line_refs(old_text, map, max_old);
        free(old_text);
        (*lines)[i].text = new_text;
    }

    /* Update line_number for each line */
    for (int i = 0; i < *count; i++)
        (*lines)[i].line_number = old_to_new[i];

    free(map);
    free(old_to_new);
    termio_write("OK\n");
    return 0;
}

/* Enter LINE EDIT mode for a specific line number */
static void edit_line(StoredLine **lines, int *count, int *cap, int line_num)
{
    (void)cap; /* Unused parameter */
    /* Find the line to edit */
    int edit_idx = -1;
    for (int i = 0; i < *count; i++)
    {
        if ((*lines)[i].line_number == line_num)
        {
            edit_idx = i;
            break;
        }
    }

    if (edit_idx < 0)
    {
        printf("?LINE NOT FOUND\n");
        return;
    }

    char edit_buf[1024];
    strncpy(edit_buf, (*lines)[edit_idx].text, sizeof(edit_buf) - 1);
    edit_buf[sizeof(edit_buf) - 1] = '\0';

    /* Use termio_lineedit for full editing capability */
    int len = termio_lineedit(line_num, edit_buf, sizeof(edit_buf));

    if (len >= 0)
    {
        /* Update the line in storage */
        free((*lines)[edit_idx].text);
        (*lines)[edit_idx].text = (char *)malloc(strlen(edit_buf) + 1);
        strcpy((*lines)[edit_idx].text, edit_buf);
        printf("OK\n");
    }
    else
    {
        printf("EDIT CANCELLED\n");
    }
}

static void list_program(StoredLine *lines, int count)
{
    for (int i = 0; i < count; i++)
    {
        termio_printf("%d %s\n", lines[i].line_number, lines[i].text ? lines[i].text : "");
    }
}

static char *build_program_text(StoredLine *lines, int count)
{
    size_t total = 0;
    for (int i = 0; i < count; i++)
    {
        total += 16 + (lines[i].text ? strlen(lines[i].text) : 0) + 2;
    }

    char *buf = xmalloc(total + 1);
    buf[0] = '\0';
    for (int i = 0; i < count; i++)
    {
        char linebuf[32];
        snprintf(linebuf, sizeof(linebuf), "%d ", lines[i].line_number);
        strcat(buf, linebuf);
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

        char *p = buffer;
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0' || *p == '!')
            continue;

        if (!isdigit((unsigned char)*p))
            continue;

        int line_num = (int)strtol(p, &p, 10);
        while (*p && isspace((unsigned char)*p))
            p++;

        insert_line(lines, count, cap, line_num, p);
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
        if (fprintf(fp, "%d %s\n", lines[i].line_number, lines[i].text ? lines[i].text : "") < 0)
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
    /* Validate line numbers */
    int start_idx = find_line_index(g_save_lines, g_save_line_count, start_line);
    int end_idx = find_line_index(g_save_lines, g_save_line_count, end_line);

    /* The end line must exist */
    if (end_idx < 0)
    {
        return -1; /* Line not found */
    }

    /* If start_line is greater than first line, we need to find it */
    if (start_idx < 0)
    {
        /* Find the insertion point and scan forward from there */
        int insert_idx = -start_idx - 1;
        if (insert_idx >= g_save_line_count || g_save_lines[insert_idx].line_number > end_line)
        {
            return -1; /* No lines in range */
        }
        start_idx = insert_idx;
    }

    end_idx = find_line_index(g_save_lines, g_save_line_count, end_line);
    if (end_idx < 0)
    {
        return -1;
    }

    /* Delete lines from start_idx to end_idx (inclusive) */
    int num_to_delete = end_idx - start_idx + 1;
    for (int i = 0; i < num_to_delete; i++)
    {
        if (start_idx < g_save_line_count)
        {
            free(g_save_lines[start_idx].text);
            memmove(&g_save_lines[start_idx], &g_save_lines[start_idx + 1],
                    (size_t)(g_save_line_count - start_idx - 1) * sizeof(StoredLine));
            g_save_line_count--;
        }
    }

    return 0;
}

static int merge_callback(const char *filename)
{
    if (!filename)
    {
        return -1;
    }

    /* Read file */
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        termio_printf("?FILE NOT FOUND\n");
        return -1;
    }

    char line_buf[1024];
    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        /* Remove trailing newline */
        size_t len = strlen(line_buf);
        if (len > 0 && line_buf[len - 1] == '\n')
        {
            line_buf[len - 1] = '\0';
        }

        if (strlen(line_buf) == 0)
        {
            continue; /* Skip empty lines */
        }

        /* Parse line number */
        char *p = line_buf;
        while (*p && isspace((unsigned char)*p))
            p++;

        if (!isdigit((unsigned char)*p))
        {
            termio_printf("?SYNTAX ERROR IN MERGE FILE\n");
            fclose(fp);
            return -1;
        }

        int line_num = 0;
        while (*p && isdigit((unsigned char)*p))
        {
            line_num = line_num * 10 + (*p - '0');
            p++;
        }

        /* Find existing line */
        int idx = find_line_index(g_save_lines, g_save_line_count, line_num);
        if (idx >= 0)
        {
            /* Line exists - overwrite it */
            termio_printf("MERGE: line %d overwritten\n", line_num);
            free(g_save_lines[idx].text);
            g_save_lines[idx].text = xstrdup(line_buf);
        }
        else
        {
            /* Line doesn't exist - add it */
            if (g_save_line_count >= g_save_lines_cap)
            {
                g_save_lines_cap = (g_save_lines_cap > 0) ? g_save_lines_cap * 2 : 256;
                g_save_lines = xrealloc(g_save_lines, (size_t)g_save_lines_cap * sizeof(StoredLine));
            }

            int insert_idx = -idx - 1;

            /* Shift lines to make room */
            if (insert_idx < g_save_line_count)
            {
                memmove(&g_save_lines[insert_idx + 1], &g_save_lines[insert_idx],
                        (size_t)(g_save_line_count - insert_idx) * sizeof(StoredLine));
            }

            g_save_lines[insert_idx].line_number = line_num;
            g_save_lines[insert_idx].text = xstrdup(line_buf);
            g_save_line_count++;
        }
    }

    fclose(fp);

    /* Clear all variables and arrays (as per MERGE semantics) */
    if (g_runtime)
    {
        runtime_clear_all(g_runtime);
    }

    /* Close all open files */
    if (g_runtime)
    {
        for (int i = 0; i < 10; i++)
        {
            runtime_close_file(g_runtime, i + 1);
        }
    }

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

static int prompt_memory_size(void)
{
    char input[64];
    int memory_size = 32768;

    termio_write("MEMORY SIZE? ");

    int len = termio_readline(input, sizeof(input));
    if (len < 0)
        return memory_size;

    char *p = input;
    while (*p && isspace((unsigned char)*p))
        p++;

    if (*p == '\0')
        return memory_size;

    char *end = NULL;
    long value = strtol(p, &end, 10);
    if (end != p && value > 0)
    {
        long bytes = value * 1024L;
        if (bytes > 0)
        {
            memory_size = (int)bytes;
        }
    }

    return memory_size;
}

static void run_interactive(void)
{
    StoredLine *lines = NULL;
    int line_count = 0;
    int line_cap = 0;
    int auto_mode = 0;
    int auto_next = 10;
    int auto_inc = 10;

    if (!termio_init(80, 24, 1))
    {
        fprintf(stderr, "Failed to initialize terminal window.\n");
        return;
    }
    termio_set_title("TRS-80 BASIC");

    int memory_size = prompt_memory_size();
    termio_write("RADIO SHACK LEVEL II BASIC\n");

    RuntimeState *runtime = runtime_create();
    runtime_set_memory_size(runtime, memory_size);
    executor_set_interrupt_flag(&g_interrupt);

    signal(SIGINT, handle_sigint);

    char input[1024];
    int copyright_shown = 0;
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

        if (auto_mode)
        {
            termio_printf("%d ", auto_next);
        }

        else
        {
            if (!copyright_shown)
            {
                termio_write("(c) 1978 Tandy Corporation\n");
                copyright_shown = 1;
            }
            termio_write("READY\n> ");
        }

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

        if (len < 0)
        {
            termio_write("\n");
            break;
        }

        char *p = input;
        while (*p && isspace((unsigned char)*p))
            p++;

        if (*p == '\0')
        {
            if (auto_mode)
            {
                auto_mode = 0;
            }
            continue;
        }

        /* Check if this is an EDIT command from scrollback browse */
        if (strncmp(p, "EDIT ", 5) == 0)
        {
            int edit_line_num = atoi(p + 5);
            if (edit_line_num > 0)
            {
                edit_line(&lines, &line_count, &line_cap, edit_line_num);
            }
            continue;
        }

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
            p += 4;
            while (*p && isspace((unsigned char)*p))
                p++;
            if (isdigit((unsigned char)*p))
            {
                int line_num = atoi(p);
                edit_line(&lines, &line_count, &line_cap, line_num);
            }
            else
            {
                printf("?SYNTAX ERROR\n");
            }
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

        if (starts_with_keyword(p, "RENUM"))
        {
            char *q = p + 5;
            while (*q == ' ' || *q == '\t')
                q++;
            int start = 10, increment = 10;
            if (*q != '\0' && *q != '\n')
            {
                start = (int)strtol(q, &q, 10);
                if (start < 1)
                    start = 10;
                while (*q == ' ' || *q == '\t')
                    q++;
                if (*q == ',')
                {
                    q++;
                    while (*q == ' ' || *q == '\t')
                        q++;
                    increment = (int)strtol(q, &q, 10);
                    if (increment < 1)
                        increment = 10;
                }
            }
            do_renum(&lines, &line_count, &line_cap, start, increment);
            continue;
        }

        if (starts_with_keyword(p, "CLEAR"))
        {
            runtime_free(runtime);
            runtime = runtime_create();
            runtime_set_memory_size(runtime, memory_size);
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

            /* Parse optional line number argument */
            int start_line_num = -1;
            if (*rest != '\0' && isdigit((unsigned char)*rest))
            {
                start_line_num = atoi(rest);
            }

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
                run_program_text_from_line(runtime, program_text, start_line_num);
                chdir(saved_cwd); /* Restore original directory */
            }
            else
            {
                run_program_text_from_line(runtime, program_text, start_line_num);
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
            char *q = p + 4;
            if (*q == '\0' || isspace((unsigned char)*q) || isdigit((unsigned char)*q) || *q == ',')
            {
                p = q;
                while (*p && isspace((unsigned char)*p))
                    p++;

                if (*p == '\0')
                {
                    auto_next = 10;
                    auto_inc = 10;
                    auto_mode = 1;
                    continue;
                }

                auto_next = 10;
                auto_inc = 10;

                if (*p == ',')
                {
                    p++;
                    auto_inc = (int)strtol(p, NULL, 10);
                }
                else if (*p != '\0')
                {
                    auto_next = (int)strtol(p, &p, 10);
                    if (*p == ',')
                    {
                        p++;
                        auto_inc = (int)strtol(p, NULL, 10);
                    }
                }

                auto_mode = 1;
                continue;
            }
        }

        if (auto_mode && !isdigit((unsigned char)*p))
        {
            insert_line(&lines, &line_count, &line_cap, auto_next, p);
            runtime_set_last_entered_line(runtime, auto_next);
            auto_next += auto_inc;
            continue;
        }

        if (isdigit((unsigned char)*p))
        {
            int line_num = (int)strtol(p, &p, 10);
            while (*p && isspace((unsigned char)*p))
                p++;
            insert_line(&lines, &line_count, &line_cap, line_num, p);
            runtime_set_last_entered_line(runtime, line_num);
            continue;
        }

        {
            char *temp = xmalloc(strlen(p) + 8);
            sprintf(temp, "0 %s\n", p);
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

    FILE *f = fopen(file_to_run, "r");
    if (f == NULL)
    {
        termio_printf("ERROR: Cannot open file '%s'\n", file_to_run);
        termio_printf("fopen: %s\n", strerror(errno));
        termio_printf("\nUsage: basicpp <filename.bas|filename.basicpp>\n");
        compat_free(g_compat_state);
        g_compat_state = NULL;
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *program_text = xmalloc(size + 1);
    if (fread(program_text, 1, size, f) != (size_t)size)
    {
        termio_printf("Error reading file\n");
        fclose(f);
        free(program_text);
        return 1;
    }
    fclose(f);
    program_text[size] = '\0';

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
        return 1;
    }

    RuntimeState *runtime = runtime_create();

    StoredLine *lines = NULL;
    int line_count = 0;
    int line_cap = 0;
    if (load_program_file(&lines, &line_count, &line_cap, file_to_run) == 0)
    {
        g_save_lines = lines;
        g_save_line_count = line_count;
        g_save_lines_cap = line_cap;
        g_runtime = runtime;
        runtime_set_save_callback(runtime, save_callback);
        runtime_set_delete_callback(runtime, delete_callback);
        runtime_set_merge_callback(runtime, merge_callback);
    }

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
