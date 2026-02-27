#include "builtins.h"
#include "eval.h"
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/time.h>
#include "termio.h"

/* INKEY$ persistent terminal state */
static struct termios inkey_saved_term;
static int inkey_term_set = 0;

static void restore_inkey_terminal(void)
{
    if (inkey_term_set)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &inkey_saved_term);
    }
}

static int setup_inkey_raw_mode(void)
{
    if (!isatty(STDIN_FILENO))
    {
        return 0;
    }

    struct termios raw;
    if (!inkey_term_set)
    {
        if (tcgetattr(STDIN_FILENO, &inkey_saved_term) != 0)
        {
            return 0;
        }
        inkey_term_set = 1;
        atexit(restore_inkey_terminal);
    }

    raw = inkey_saved_term;
    raw.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL);
    raw.c_iflag &= ~(ICRNL | INLCR | IGNCR | IXON | IXOFF);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
    {
        return 0;
    }

    return 1;
}

/* Helper to safely get numeric argument */
static double get_numeric_arg(RuntimeState *state, ASTExpr **args, int num_args, int index)
{
    if (index >= 0 && index < num_args && args[index] != NULL)
    {
        return eval_numeric_expr(state, args[index]);
    }
    return 0.0;
}

/* Helper to safely get string argument */
static char *get_string_arg(RuntimeState *state, ASTExpr **args, int num_args, int index)
{
    if (index >= 0 && index < num_args && args[index] != NULL)
    {
        return eval_string_expr(state, args[index]);
    }
    return xstrdup("");
}

double call_numeric_function(RuntimeState *state, const char *func_name,
                             ASTExpr **args, int num_args)
{
    if (func_name == NULL)
    {
        return 0.0;
    }

    /* Check for user-defined functions first */
    ASTExpr *user_body = runtime_get_function_body(state, func_name);
    if (user_body != NULL)
    {
        int param_count = runtime_get_function_param_count(state, func_name);
        const char **params = runtime_get_function_params(state, func_name);

        /* Evaluate arguments and set as parameter variables */
        double *arg_values = (param_count > 0) ? xmalloc(param_count * sizeof(double)) : NULL;
        for (int i = 0; i < param_count && i < num_args; i++)
        {
            arg_values[i] = eval_numeric_expr(state, args[i]);
        }
        for (int i = num_args; i < param_count; i++)
        {
            arg_values[i] = 0.0;
        }

        /* Set parameters as variables */
        for (int i = 0; i < param_count; i++)
        {
            runtime_set_variable(state, params[i], arg_values[i]);
        }

        /* Evaluate the function body */
        double result = eval_numeric_expr(state, user_body);

        /* Clean up temporary parameters */
        if (arg_values)
        {
            free(arg_values);
        }

        return result;
    }

    /* Math functions */
    if (strcmp(func_name, "ABS") == 0)
    {
        return fabs(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "SIN") == 0)
    {
        return sin(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "COS") == 0)
    {
        return cos(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "TAN") == 0)
    {
        return tan(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "ATN") == 0)
    {
        return atan(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "EXP") == 0)
    {
        return exp(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "LOG") == 0)
    {
        double arg = get_numeric_arg(state, args, num_args, 0);
        return (arg > 0) ? log10(arg) : 0.0;
    }
    else if (strcmp(func_name, "LN") == 0)
    {
        double arg = get_numeric_arg(state, args, num_args, 0);
        return (arg > 0) ? log(arg) : 0.0;
    }
    else if (strcmp(func_name, "SQR") == 0)
    {
        double arg = get_numeric_arg(state, args, num_args, 0);
        return (arg >= 0) ? sqrt(arg) : 0.0;
    }
    else if (strcmp(func_name, "INT") == 0)
    {
        return floor(get_numeric_arg(state, args, num_args, 0));
    }
    else if (strcmp(func_name, "SGN") == 0)
    {
        double arg = get_numeric_arg(state, args, num_args, 0);
        return (arg > 0) ? 1.0 : (arg < 0) ? -1.0
                                           : 0.0;
    }
    else if (strcmp(func_name, "RND") == 0)
    {
        double arg = get_numeric_arg(state, args, num_args, 0);

        if (arg == 0.0)
        {
            /* RND(0) - return last value */
            return runtime_get_last_rnd(state);
        }
        else if (arg < 0.0)
        {
            /* RND(negative) - reseed with the absolute value */
            int seed = (int)(-arg) & 0xFFFF; /* Ensure 16-bit */
            runtime_randomize(state, seed);
            /* Return the newly generated value */
            return runtime_random(state);
        }
        else
        {
            /* RND(positive) - generate next random number */
            return runtime_random(state);
        }
    }
    else if (strcmp(func_name, "VAL") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        double result = atof(str);
        free(str);
        return result;
    }
    else if (strcmp(func_name, "ASC") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        double result = (str && str[0]) ? (double)(unsigned char)str[0] : 0.0;
        free(str);
        return result;
    }
    else if (strcmp(func_name, "LEN") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        double result = (double)strlen(str);
        free(str);
        return result;
    }
    else if (strcmp(func_name, "INSTR") == 0)
    {
        /* INSTR(string, substring) or INSTR(start, string, substring)
         * Returns position of substring in string (1-based), or 0 if not found
         */
        int start_pos = 1;
        char *string = NULL;
        char *substring = NULL;

        if (num_args == 2)
        {
            /* INSTR(string, substring) */
            string = get_string_arg(state, args, num_args, 0);
            substring = get_string_arg(state, args, num_args, 1);
            start_pos = 1;
        }
        else if (num_args >= 3)
        {
            /* INSTR(start, string, substring) */
            start_pos = (int)get_numeric_arg(state, args, num_args, 0);
            string = get_string_arg(state, args, num_args, 1);
            substring = get_string_arg(state, args, num_args, 2);
        }
        else
        {
            /* Invalid arguments */
            return 0.0;
        }

        /* Ensure start_pos is valid (1-based) */
        if (start_pos < 1)
            start_pos = 1;

        /* Adjust to 0-based for C string operations */
        int str_len = strlen(string);
        int sub_len = strlen(substring);
        int search_start = start_pos - 1;

        if (search_start >= str_len || sub_len == 0 || search_start < 0)
        {
            free(string);
            free(substring);
            return 0.0;
        }

        /* Search for substring */
        const char *found = strstr(string + search_start, substring);
        double result = (found != NULL) ? (double)(found - string + 1) : 0.0;

        free(string);
        free(substring);
        return result;
    }
    else if (strcmp(func_name, "PEEK") == 0)
    {
        int addr = (int)get_numeric_arg(state, args, num_args, 0);
        return (double)runtime_peek(state, addr);
    }
    else if (strcmp(func_name, "POINT") == 0)
    {
        /* POINT function (CoCo graphics) - not supported in Basic++ */
        return 0.0;
    }
    else if (strcmp(func_name, "EOF") == 0)
    {
        int handle = (int)get_numeric_arg(state, args, num_args, 0);
        return runtime_file_eof(state, handle) ? -1.0 : 0.0;
    }
    else if (strcmp(func_name, "LOC") == 0)
    {
        int handle = (int)get_numeric_arg(state, args, num_args, 0);
        return (double)runtime_file_loc(state, handle);
    }
    else if (strcmp(func_name, "LOF") == 0)
    {
        int handle = (int)get_numeric_arg(state, args, num_args, 0);
        return (double)runtime_file_lof(state, handle);
    }
    else if (strcmp(func_name, "VARPTR") == 0)
    {
        if (num_args > 0 && args[0] && args[0]->type == EXPR_VAR && args[0]->var_name)
        {
            const char *var_name = args[0]->var_name;
            int hash = 0;
            for (int i = 0; var_name[i]; i++)
            {
                hash = (hash * 31 + (unsigned char)var_name[i]) % (32768 / 2);
            }
            return (double)(32768 / 2 + hash);
        }
        return 0.0;
    }
    else if (strcmp(func_name, "GETA") == 0)
    {
        return (double)runtime_get_reg_a(state);
    }
    else if (strcmp(func_name, "GETB") == 0)
    {
        return (double)runtime_get_reg_b(state);
    }
    else if (strcmp(func_name, "USR") == 0)
    {
        int addr = (num_args > 0) ? (int)get_numeric_arg(state, args, num_args, 0)
                                  : runtime_get_usr_address(state);

        int a = runtime_get_reg_a(state);
        int b = runtime_get_reg_b(state);

        switch (addr)
        {
        case 1000:
            return (double)(a + b);
        case 1100:
            return (double)(a - b);
        case 1200:
            return (double)(a * b);
        case 1300:
            return (double)(a * a);
        case 1400:
            return (double)(-a);
        case 1500:
            return (double)(a < 0 ? -a : a);
        case 1600:
            return 20260128.0;
        default:
            return (double)runtime_peek(state, addr);
        }
    }
    else if (strcmp(func_name, "FRE") == 0)
    {
        /* Free memory - return a large value */
        return 65000.0;
    }
    else if (strcmp(func_name, "POS") == 0)
    {
        /* Cursor position - stub */
        return 0.0;
    }

    /* Unknown function */
    return 0.0;
}

char *call_string_function(RuntimeState *state, const char *func_name,
                           ASTExpr **args, int num_args)
{
    if (func_name == NULL)
    {
        return xstrdup("");
    }

    /* Check for user-defined functions first */
    ASTExpr *user_body = runtime_get_function_body(state, func_name);
    if (user_body != NULL)
    {
        int param_count = runtime_get_function_param_count(state, func_name);
        const char **params = runtime_get_function_params(state, func_name);

        /* Evaluate arguments and set as parameter variables */
        double *arg_values = (param_count > 0) ? xmalloc(param_count * sizeof(double)) : NULL;
        for (int i = 0; i < param_count && i < num_args; i++)
        {
            arg_values[i] = eval_numeric_expr(state, args[i]);
        }
        for (int i = num_args; i < param_count; i++)
        {
            arg_values[i] = 0.0;
        }

        /* Set parameters as variables */
        for (int i = 0; i < param_count; i++)
        {
            runtime_set_variable(state, params[i], arg_values[i]);
        }

        /* Evaluate the function body and convert to string */
        double result = eval_numeric_expr(state, user_body);

        /* Clean up temporary parameters */
        if (arg_values)
        {
            free(arg_values);
        }

        /* Convert numeric result to string */
        char buf[64];
        if (fabs(result) < 1e-10 && result != 0.0)
            snprintf(buf, sizeof(buf), "%.9e", result);
        else
            snprintf(buf, sizeof(buf), "%.15g", result);
        return xstrdup(buf);
    }

    if (strcmp(func_name, "INKEY$") == 0)
    {
        char *result = xmalloc(2);
        result[0] = '\0';
        result[1] = '\0';

        int ch = EOF;

#ifdef USE_SDL
        ch = termio_poll_key();
        if (ch != -1)
        {
            if (ch >= 32 && ch < 127)
                result[0] = (char)ch;
            else if (ch == '\n' || ch == '\r')
                result[0] = '\n';
        }
        return result;
#endif
        if (setup_inkey_raw_mode())
        {
            fd_set readfds;
            struct timeval timeout;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 50000;

            if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0)
            {
                if (FD_ISSET(STDIN_FILENO, &readfds))
                {
                    unsigned char buf;
                    if (read(STDIN_FILENO, &buf, 1) == 1)
                    {
                        ch = (int)buf;
                    }
                }
            }
        }
        else
        {
            fd_set readfds;
            struct timeval timeout;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0)
            {
                if (FD_ISSET(STDIN_FILENO, &readfds))
                {
                    unsigned char buf;
                    if (read(STDIN_FILENO, &buf, 1) == 1)
                    {
                        ch = (int)buf;
                    }
                }
            }
        }

        if (ch != EOF && ch != 0)
        {
            if (ch >= 32 && ch < 127)
            {
                result[0] = (char)ch;
            }
            else if (ch == '\n' || ch == '\r')
            {
                result[0] = '\n';
            }
        }

        if (inkey_term_set)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &inkey_saved_term);
        }

        return result;
    }
    else if (strcmp(func_name, "CHR$") == 0)
    {
        int code = (int)get_numeric_arg(state, args, num_args, 0);
        if (code >= 0 && code <= 255)
        {
            char result[2] = {(char)code, '\0'};
            return xstrdup(result);
        }
        return xstrdup("");
    }
    else if (strcmp(func_name, "STR$") == 0)
    {
        double num = get_numeric_arg(state, args, num_args, 0);
        char buf[64];
        snprintf(buf, sizeof(buf), "%.15g", num);
        return xstrdup(buf);
    }
    else if (strcmp(func_name, "LEFT$") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        int len = (int)get_numeric_arg(state, args, num_args, 1);
        if (len < 0)
            len = 0;
        int str_len = strlen(str);
        if (len > str_len)
            len = str_len;

        char *result = xmalloc(len + 1);
        strncpy(result, str, len);
        result[len] = '\0';
        free(str);
        return result;
    }
    else if (strcmp(func_name, "RIGHT$") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        int len = (int)get_numeric_arg(state, args, num_args, 1);
        int str_len = strlen(str);
        if (len < 0)
            len = 0;
        if (len > str_len)
            len = str_len;

        char *result = xstrdup(str + str_len - len);
        free(str);
        return result;
    }
    else if (strcmp(func_name, "MID$") == 0)
    {
        char *str = get_string_arg(state, args, num_args, 0);
        int start = (int)get_numeric_arg(state, args, num_args, 1) - 1; /* BASIC is 1-indexed */
        int len = (num_args >= 3) ? (int)get_numeric_arg(state, args, num_args, 2) : 32767;

        int str_len = strlen(str);
        if (start < 0)
            start = 0;
        if (start >= str_len)
        {
            free(str);
            return xstrdup("");
        }
        if (len < 0)
            len = 0;
        if (start + len > str_len)
            len = str_len - start;

        char *result = xmalloc(len + 1);
        strncpy(result, str + start, len);
        result[len] = '\0';
        free(str);
        return result;
    }
    else if (strcmp(func_name, "STRING$") == 0)
    {
        int count = (int)get_numeric_arg(state, args, num_args, 0);
        if (count < 0)
            count = 0;
        if (count > 255)
            count = 255;

        /* Second arg can be string or ASCII code */
        char ch;
        if (num_args >= 2 && args[1] != NULL)
        {
            if (args[1]->type == EXPR_STRING && args[1]->str_value && args[1]->str_value[0])
            {
                ch = args[1]->str_value[0];
            }
            else
            {
                int code = (int)get_numeric_arg(state, args, num_args, 1);
                ch = (code >= 0 && code <= 255) ? (char)code : ' ';
            }
        }
        else
        {
            ch = ' ';
        }

        char *result = xmalloc(count + 1);
        memset(result, ch, count);
        result[count] = '\0';
        return result;
    }
    else if (strcmp(func_name, "SPACE$") == 0)
    {
        int count = (int)get_numeric_arg(state, args, num_args, 0);
        if (count < 0)
            count = 0;
        if (count > 255)
            count = 255;

        char *result = xmalloc(count + 1);
        memset(result, ' ', count);
        result[count] = '\0';
        return result;
    }

    /* Unknown string function */
    return xstrdup("");
}
