#include "trace.h"
#include "executor.h"
#include "termio.h"
#include "common.h"
#include "ast.h"
#include "errors.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Global trace state */
TraceState g_trace_state = {
    .enabled = 0,
    .step_mode = 0,
    .breakpoint_count = 0,
    .break_on_next = 0};

void trace_init(void)
{
    g_trace_state.enabled = 0;
    g_trace_state.step_mode = 0;
    g_trace_state.breakpoint_count = 0;
    g_trace_state.break_on_next = 0;
    memset(g_trace_state.breakpoints, 0, sizeof(g_trace_state.breakpoints));
}

void trace_enable(void)
{
    g_trace_state.enabled = 1;
    termio_printf("Trace ON\n");
}

void trace_disable(void)
{
    g_trace_state.enabled = 0;
    termio_printf("Trace OFF\n");
}

void trace_step(void)
{
    g_trace_state.enabled = 1;
    g_trace_state.step_mode = 1;
    termio_printf("Step mode enabled\n");
}

void trace_continue(void)
{
    g_trace_state.enabled = 0;
    g_trace_state.step_mode = 0;
    termio_printf("Continuing...\n");
}

int trace_add_breakpoint(int line_number)
{
    if (g_trace_state.breakpoint_count >= 256)
    {
        termio_printf("?Breakpoint limit exceeded\n");
        return -1;
    }

    /* Check if already exists */
    for (int i = 0; i < g_trace_state.breakpoint_count; i++)
    {
        if (g_trace_state.breakpoints[i] == line_number)
        {
            termio_printf("?Breakpoint already set at line %d\n", line_number);
            return -1;
        }
    }

    g_trace_state.breakpoints[g_trace_state.breakpoint_count++] = line_number;
    termio_printf("Breakpoint set at line %d\n", line_number);
    return 0;
}

int trace_remove_breakpoint(int line_number)
{
    for (int i = 0; i < g_trace_state.breakpoint_count; i++)
    {
        if (g_trace_state.breakpoints[i] == line_number)
        {
            /* Shift remaining breakpoints */
            for (int j = i; j < g_trace_state.breakpoint_count - 1; j++)
            {
                g_trace_state.breakpoints[j] = g_trace_state.breakpoints[j + 1];
            }
            g_trace_state.breakpoint_count--;
            termio_printf("Breakpoint removed at line %d\n", line_number);
            return 0;
        }
    }

    termio_printf("?No breakpoint at line %d\n", line_number);
    return -1;
}

int trace_has_breakpoint(int line_number)
{
    for (int i = 0; i < g_trace_state.breakpoint_count; i++)
    {
        if (g_trace_state.breakpoints[i] == line_number)
        {
            return 1;
        }
    }
    return 0;
}

void trace_clear_breakpoints(void)
{
    g_trace_state.breakpoint_count = 0;
    termio_printf("All breakpoints cleared\n");
}

void trace_list_breakpoints(void)
{
    if (g_trace_state.breakpoint_count == 0)
    {
        termio_printf("No breakpoints set\n");
        return;
    }

    termio_printf("Breakpoints:\n");
    for (int i = 0; i < g_trace_state.breakpoint_count; i++)
    {
        termio_printf("  %d\n", g_trace_state.breakpoints[i]);
    }
}

/* Convert statement to readable form */
static void trace_print_statement(int line_number, ASTStmt *stmt)
{
    if (!stmt)
    {
        termio_printf("Line %d: [unknown statement]\n", line_number);
        return;
    }

    switch (stmt->type)
    {
    case STMT_PRINT:
        termio_printf("Line %d: PRINT", line_number);
        if (stmt->num_exprs > 0)
        {
            termio_printf(" (expression)\n");
        }
        else
        {
            termio_printf("\n");
        }
        break;

    case STMT_LET:
        if (stmt->num_exprs >= 2)
        {
            /* First expr is variable name (if EXPR_VAR), second is value */
            termio_printf("Line %d: LET %s = (expression)\n",
                          line_number,
                          stmt->exprs[0]->var_name ? stmt->exprs[0]->var_name : "var");
        }
        else
        {
            termio_printf("Line %d: LET\n", line_number);
        }
        break;

    case STMT_INPUT:
        termio_printf("Line %d: INPUT", line_number);
        if (stmt->num_exprs > 0)
        {
            termio_printf(" (variables)\n");
        }
        else
        {
            termio_printf("\n");
        }
        break;

    case STMT_IF:
        termio_printf("Line %d: IF (condition) THEN\n", line_number);
        break;

    case STMT_FOR:
        termio_printf("Line %d: FOR (loop)\n", line_number);
        break;

    case STMT_NEXT:
        termio_printf("Line %d: NEXT\n", line_number);
        break;

    case STMT_GOSUB:
        if (stmt->num_exprs > 0 && stmt->exprs[0])
        {
            termio_printf("Line %d: GOSUB %d\n", line_number, (int)stmt->exprs[0]->num_value);
        }
        else
        {
            termio_printf("Line %d: GOSUB\n", line_number);
        }
        break;

    case STMT_GOTO:
        if (stmt->num_exprs > 0 && stmt->exprs[0])
        {
            termio_printf("Line %d: GOTO %d\n", line_number, (int)stmt->exprs[0]->num_value);
        }
        else
        {
            termio_printf("Line %d: GOTO\n", line_number);
        }
        break;

    case STMT_RETURN:
        termio_printf("Line %d: RETURN\n", line_number);
        break;

    case STMT_END:
        termio_printf("Line %d: END\n", line_number);
        break;

    case STMT_CASE:
        termio_printf("Line %d: CASE (statement)\n", line_number);
        break;

    case STMT_DO_LOOP:
        termio_printf("Line %d: DO...LOOP\n", line_number);
        break;

    case STMT_MERGE:
        termio_printf("Line %d: MERGE\n", line_number);
        break;

    default:
        termio_printf("Line %d: [%d]\n", line_number, stmt->type);
        break;
    }
}

void trace_print_variables(RuntimeState *runtime)
{
    if (!runtime || runtime_get_var_count(runtime) == 0)
    {
        termio_printf("  [no variables]\n");
        return;
    }

    int count = runtime_get_var_count(runtime);
    /* Limit display to first 30 variables to avoid clutter */
    int display_count = count < 30 ? count : 30;

    for (int i = 0; i < display_count; i++)
    {
        RuntimeVar rv;
        if (runtime_get_var_by_index(runtime, i, &rv) < 0)
        {
            break;
        }

        termio_printf("  %s = ", rv.name);

        if (rv.is_array)
        {
            termio_printf("[array]\n");
        }
        else if (rv.is_string)
        {
            termio_printf("\"%s\"\n", rv.string_value ? rv.string_value : "");
        }
        else
        {
            /* Format numeric value */
            double val = rv.numeric_value;
            if (val == (int)val)
            {
                termio_printf("%d\n", (int)val);
            }
            else
            {
                termio_printf("%g\n", val);
            }
        }
    }

    if (count > 30)
    {
        termio_printf("  ... and %d more variables\n", count - 30);
    }
}

void trace_print_variable(RuntimeState *runtime, const char *name)
{
    if (!runtime || !name)
    {
        return;
    }

    int count = runtime_get_var_count(runtime);
    for (int i = 0; i < count; i++)
    {
        RuntimeVar rv;
        if (runtime_get_var_by_index(runtime, i, &rv) < 0)
        {
            continue;
        }

        if (strcasecmp(rv.name, name) == 0)
        {
            termio_printf("%s = ", rv.name);
            if (rv.is_array)
            {
                termio_printf("[array]\n");
            }
            else if (rv.is_string)
            {
                termio_printf("\"%s\"\n", rv.string_value ? rv.string_value : "");
            }
            else
            {
                double val = rv.numeric_value;
                if (val == (int)val)
                {
                    termio_printf("%d\n", (int)val);
                }
                else
                {
                    termio_printf("%g\n", val);
                }
            }
            return;
        }
    }

    termio_printf("?Variable %s not found\n", name);
}

/* Trace pause and user interaction */
static void trace_pause(ExecutionContext *ctx, int line_number)
{
    int keep_tracing = 1;
    char input_buf[256];

    while (keep_tracing)
    {
        /* Display prompt and get user input */
        termio_printf("[TRACE] (S=step, C=continue, V=vars, L=list breaks, Q=quit): ");

        if (termio_readline(input_buf, sizeof(input_buf)) <= 0)
        {
            /* EOF or error - auto-continue */
            g_trace_state.enabled = 0;
            g_trace_state.step_mode = 0;
            break;
        }

        /* Get first character */
        char cmd = toupper((unsigned char)input_buf[0]);

        if (cmd == 0 || cmd == '\n')
        {
            /* Empty input, show help */
            termio_printf("Commands: S=step, C=continue, V=vars, L=list breaks, Q=quit\n");
            continue;
        }

        switch (cmd)
        {
        case 'S':
            /* Step to next statement */
            g_trace_state.step_mode = 1;
            keep_tracing = 0;
            break;

        case 'C':
            /* Continue without tracing */
            g_trace_state.enabled = 0;
            g_trace_state.step_mode = 0;
            keep_tracing = 0;
            break;

        case 'V':
            /* Show variables */
            termio_printf("Variables at line %d:\n", line_number);
            trace_print_variables(executor_get_runtime(ctx));
            termio_printf("\n");
            break;

        case 'L':
            /* List breakpoints */
            trace_list_breakpoints();
            termio_printf("\n");
            break;

        case 'Q':
            /* Quit execution */
            runtime_set_error(executor_get_runtime(ctx), BASIC_ERR_SYNTAX_ERROR, line_number);
            keep_tracing = 0;
            break;

        default:
            /* Invalid command */
            termio_printf("Invalid command '%c'. Try S/C/V/L/Q\n", cmd);
            break;
        }
    }
}

/* Called before executing each statement */
void trace_before_statement(struct ExecutionContext *ctx, int line_number, ASTStmt *stmt)
{
    RuntimeState *runtime = executor_get_runtime(ctx);
    if (!ctx || !runtime)
    {
        return;
    }

    /* Check if we should trace this statement */
    int should_trace = g_trace_state.enabled ||
                       g_trace_state.step_mode ||
                       trace_has_breakpoint(line_number);

    if (!should_trace)
    {
        return;
    }

    /* Print statement information */
    termio_write("\n");
    trace_print_statement(line_number, stmt);

    /* Print current variables */
    termio_printf("Variables:\n");
    trace_print_variables(runtime);

    /* Pause for user interaction */
    trace_pause(ctx, line_number);

    /* If step mode, disable after this statement */
    if (g_trace_state.step_mode)
    {
        g_trace_state.step_mode = 0;
    }
}
