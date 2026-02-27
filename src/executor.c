#include "executor.h"
#include "ast.h"
#include "runtime.h"
#include "eval.h"
#include "builtins.h"
#include "common.h"
#include "errors.h"
#include "termio.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

static volatile sig_atomic_t *g_interrupt_flag = NULL;

void executor_set_interrupt_flag(volatile sig_atomic_t *flag)
{
    g_interrupt_flag = flag;
}

int executor_check_interrupt(void)
{
    if (g_interrupt_flag && *g_interrupt_flag)
    {
        *g_interrupt_flag = 0;
        return 1;
    }
    return 0;
}

/* Trigger interrupt from external event (e.g., SDL Ctrl+C) */
void executor_trigger_interrupt(void)
{
    if (g_interrupt_flag)
    {
        *g_interrupt_flag = 1;
    }
}

/* Process SDL events periodically during loops */
void executor_process_events(void)
{
    termio_handle_events();
}

static void preload_data(RuntimeState *state, Program *prog)
{
    runtime_data_clear(state);
    if (!prog || !prog->lines)
    {
        return;
    }

    for (int i = 0; i < prog->num_lines; i++)
    {
        ASTStmt *stmt = prog->lines[i]->stmt;
        if (!stmt || stmt->type != STMT_DATA)
        {
            continue;
        }

        runtime_data_start_segment(state, prog->lines[i]->line_number);

        for (int j = 0; j < stmt->num_exprs; j++)
        {
            ASTExpr *expr = stmt->exprs[j];
            if (!expr)
                continue;
            if (expr->type == EXPR_STRING)
            {
                runtime_data_add_string(state, expr->str_value ? expr->str_value : "");
            }
            else if (expr->type == EXPR_NUMBER)
            {
                runtime_data_add_number(state, expr->num_value);
            }
        }
    }
}

/** Scope management for procedures **/

/* Create a new procedure scope */
static ProcedureScope *proc_scope_create(void)
{
    ProcedureScope *scope = xmalloc(sizeof(ProcedureScope));
    scope->var_names = NULL;
    scope->var_values = NULL;
    scope->num_vars = 0;
    scope->capacity = 0;
    return scope;
}

/* Free a procedure scope */
static void proc_scope_free(ProcedureScope *scope)
{
    if (scope)
    {
        for (int i = 0; i < scope->num_vars; i++)
        {
            free(scope->var_names[i]);
        }
        free(scope->var_names);
        free(scope->var_values);
        free(scope);
    }
}

/* Push a scope onto the scope stack */
static void proc_scope_push(ExecutionContext *ctx, ProcedureScope *scope)
{
    if (ctx->scope_sp >= ctx->scope_cap)
    {
        ctx->scope_cap = ctx->scope_cap == 0 ? 4 : ctx->scope_cap * 2;
        ctx->scope_stack = xrealloc(ctx->scope_stack, ctx->scope_cap * sizeof(ProcedureScope));
    }
    ctx->scope_stack[ctx->scope_sp++] = *scope;
    free(scope); /* Container freed, values are in stack */
}

/* Pop a scope from the scope stack */
static ProcedureScope *proc_scope_pop(ExecutionContext *ctx)
{
    if (ctx->scope_sp <= 0)
        return NULL;

    ProcedureScope *scope = xmalloc(sizeof(ProcedureScope));
    *scope = ctx->scope_stack[--ctx->scope_sp];
    return scope;
}

/* Get current scope (top of stack) */
static ProcedureScope *proc_scope_current(ExecutionContext *ctx)
{
    if (ctx->scope_sp <= 0)
        return NULL;
    return &ctx->scope_stack[ctx->scope_sp - 1];
}

/* Save a variable to the current scope */
static void proc_scope_save_var(ExecutionContext *ctx, const char *name, double value)
{
    ProcedureScope *scope = proc_scope_current(ctx);
    if (!scope)
        return;

    if (scope->num_vars >= scope->capacity)
    {
        scope->capacity = scope->capacity == 0 ? 8 : scope->capacity * 2;
        scope->var_names = xrealloc(scope->var_names, scope->capacity * sizeof(char *));
        scope->var_values = xrealloc(scope->var_values, scope->capacity * sizeof(double));
    }

    scope->var_names[scope->num_vars] = xstrdup(name);
    scope->var_values[scope->num_vars] = value;
    scope->num_vars++;
}

/* Forward declarations */

static int execute_stmt_internal(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_print_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_print_at_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_print_using_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_input_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_line_input_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_let_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_if_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_on_goto_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_for_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_next_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_goto_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_gosub_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_return_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_procedure_call_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_dim_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_read_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_data_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_restore_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_def_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_def_fn_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_on_error_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_resume_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_sleep_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_beep_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_sound_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_cls_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_clear_stmt(ExecutionContext *ctx, ASTStmt *stmt);

static int execute_delete_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_merge_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_error_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_open_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_close_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_write_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_get_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_put_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_poke_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_save_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_end_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_while_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_wend_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_do_loop_stmt(ExecutionContext *ctx, ASTStmt *stmt);
static int execute_exit_stmt(ExecutionContext *ctx, ASTStmt *stmt);

/* Find program line by line number */
int find_program_line(Program *prog, int line_number)
{
    if (prog == NULL || prog->lines == NULL)
    {
        return -1;
    }

    for (int i = 0; i < prog->num_lines; i++)
    {
        if (prog->lines[i]->line_number == line_number)
        {
            return i;
        }
    }

    return -1;
}

/* Execute a single statement */
static int execute_stmt_internal(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    runtime_set_current_state(ctx->runtime);

    int result = 0;

    switch (stmt->type)
    {
    case STMT_PRINT:
        result = execute_print_stmt(ctx, stmt);
        break;
    case STMT_PRINT_AT:
        result = execute_print_at_stmt(ctx, stmt);
        break;
    case STMT_PRINT_USING:
        result = execute_print_using_stmt(ctx, stmt);
        break;
    case STMT_INPUT:
        result = execute_input_stmt(ctx, stmt);
        break;
    case STMT_LINE_INPUT:
        result = execute_line_input_stmt(ctx, stmt);
        break;
    case STMT_LET:
        result = execute_let_stmt(ctx, stmt);
        break;
    case STMT_IF:
        result = execute_if_stmt(ctx, stmt);
        break;
    case STMT_ON_GOTO:
        result = execute_on_goto_stmt(ctx, stmt);
        break;
    case STMT_FOR:
        result = execute_for_stmt(ctx, stmt);
        break;
    case STMT_NEXT:
        result = execute_next_stmt(ctx, stmt);
        break;
    case STMT_GOTO:
        result = execute_goto_stmt(ctx, stmt);
        break;
    case STMT_GOSUB:
        result = execute_gosub_stmt(ctx, stmt);
        break;
    case STMT_RETURN:
        result = execute_return_stmt(ctx, stmt);
        break;
    case STMT_DIM:
        result = execute_dim_stmt(ctx, stmt);
        break;
    case STMT_READ:
        result = execute_read_stmt(ctx, stmt);
        break;
    case STMT_DATA:
        result = execute_data_stmt(ctx, stmt);
        break;
    case STMT_RESTORE:
        result = execute_restore_stmt(ctx, stmt);
        break;
    case STMT_DEFINT:
    case STMT_DEFSNG:
    case STMT_DEFDBL:
    case STMT_DEFSTR:
        result = execute_def_stmt(ctx, stmt);
        break;
    case STMT_DEF_FN:
        result = execute_def_fn_stmt(ctx, stmt);
        break;
    case STMT_ON_ERROR:
        result = execute_on_error_stmt(ctx, stmt);
        break;
    case STMT_RESUME:
        result = execute_resume_stmt(ctx, stmt);
        break;
    case STMT_SLEEP:
        result = execute_sleep_stmt(ctx, stmt);
        break;
    case STMT_BEEP:
        result = execute_beep_stmt(ctx, stmt);
        break;
    case STMT_CLS:
        result = execute_cls_stmt(ctx, stmt);
        break;
    case STMT_CLEAR:
        result = execute_clear_stmt(ctx, stmt);
        break;
    case STMT_DELETE:
        result = execute_delete_stmt(ctx, stmt);
        break;
    case STMT_MERGE:
        result = execute_merge_stmt(ctx, stmt);
        break;
    case STMT_ERROR:
        result = execute_error_stmt(ctx, stmt);
        break;
    case STMT_OPEN:
        result = execute_open_stmt(ctx, stmt);
        break;
    case STMT_CLOSE:
        result = execute_close_stmt(ctx, stmt);
        break;
    case STMT_WRITE:
        result = execute_write_stmt(ctx, stmt);
        break;
    case STMT_GET:
        result = execute_get_stmt(ctx, stmt);
        break;
    case STMT_PUT:
        result = execute_put_stmt(ctx, stmt);
        break;
    case STMT_POKE:
        result = execute_poke_stmt(ctx, stmt);
        break;
    case STMT_SAVE:
        result = execute_save_stmt(ctx, stmt);
        break;
    case STMT_END:
        result = execute_end_stmt(ctx, stmt);
        break;
    case STMT_REM:
        /* Comment - do nothing */
        result = 0;
        break;
    case STMT_RANDOMIZE:
        runtime_randomize(ctx->runtime, (unsigned)time(NULL));
        result = 0;
        break;
    case STMT_TRON:
        runtime_set_trace(ctx->runtime, 1);
        termio_write("TRACE ON\n");
        result = 0;
        break;
    case STMT_TROFF:
        runtime_set_trace(ctx->runtime, 0);
        termio_write("TRACE OFF\n");
        result = 0;
        break;
    case STMT_STOP:
        /* STOP: halt program execution and store stop line number */
        runtime_set_stop_state(ctx->runtime, ctx->program->lines[ctx->current_line_index]->line_number);
        termio_write("STOP\n");
        result = 1; /* Signal halt (will cause execute_program to exit) */
        break;
    case STMT_CONT:
        /* CONT: resume execution from STOP point if available */
        if (runtime_is_stopped(ctx->runtime))
        {
            int stop_line = runtime_get_stop_line(ctx->runtime);
            int stop_index = find_program_line(ctx->program, stop_line);
            if (stop_index >= 0)
            {
                runtime_clear_stop_state(ctx->runtime);
                ctx->next_line_index = stop_index;
                result = 0;
            }
            else
            {
                termio_write("Can't continue\n");
                result = 0;
            }
        }
        else
        {
            termio_write("Can't continue\n");
            result = 0;
        }
        break;
    case STMT_SOUND:
        result = execute_sound_stmt(ctx, stmt);
        break;
    case STMT_WHILE:
        result = execute_while_stmt(ctx, stmt);
        break;
    case STMT_WEND:
        result = execute_wend_stmt(ctx, stmt);
        break;
    case STMT_DO_LOOP:
        result = execute_do_loop_stmt(ctx, stmt);
        break;
    case STMT_EXIT:
        result = execute_exit_stmt(ctx, stmt);
        break;
    case STMT_COLOR:
    case STMT_PCOLOR:
    case STMT_SET:
    case STMT_RESET:
    case STMT_LINE:
    case STMT_CIRCLE:
    case STMT_PAINT:
    case STMT_SCREEN:
        result = ast_execute_stmt(stmt);
        termio_render_graphics();
        break;
    case STMT_PROCEDURE_DEF:
        /* PROCEDURE definition: store in registry for later calls */
        /* PHASE 1.3: Implement procedure registry */
        result = 0; /* For now, just ignore the definition */
        break;
    case STMT_PROCEDURE_CALL:
        /* PROCEDURE call: execute the procedure with parameters */
        result = execute_procedure_call_stmt(ctx, stmt);
        break;
    default:
        /* Unknown or unimplemented statement */
        result = 0;
        break;
    }

    /* Check for runtime error after statement execution */
    int err = runtime_get_error(ctx->runtime);
    if (err != 0)
    {
        int handler_line = runtime_get_error_handler(ctx->runtime);
        if (handler_line > 0 && !runtime_is_in_error_handler(ctx->runtime))
        {
            /* Set error handler active to prevent recursion */
            runtime_set_in_error_handler(ctx->runtime, 1);
            ctx->next_line_index = find_program_line(ctx->program, handler_line);
            ctx->skip_chained = 1;
            /* Ensure ERL is set to the line that raised the error */
            if (ctx->current_line_index >= 0 && ctx->current_line_index < ctx->program->num_lines)
                runtime_set_error(ctx->runtime, err, ctx->program->lines[ctx->current_line_index]->line_number);
            return 0;
        }
        /* Already in error handler: do not abort; let handler run (e.g. RESUME) */
        if (!runtime_is_in_error_handler(ctx->runtime))
            return -err;
    }
    if (result != 0)
    {
        return result;
    }

    if (ctx->next_stmt_override != NULL && ctx->next_line_index == ctx->current_line_index)
    {
        return 0;
    }

    if (ctx->skip_chained)
    {
        ctx->skip_chained = 0;
        return 0;
    }

    /* Execute chained statements (colon-separated on same line) */
    if (stmt->next != NULL)
    {
        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] Chain from line %d\n", ctx->program->lines[ctx->current_line_index]->line_number);
        }
        return execute_stmt_internal(ctx, stmt->next);
    }

    return 0;
}

static void ensure_for_capacity(ExecutionContext *ctx)
{
    if (ctx->for_sp >= ctx->for_cap)
    {
        ctx->for_cap = (ctx->for_cap == 0) ? 16 : ctx->for_cap * 2;
        ctx->for_stack = xrealloc(ctx->for_stack, ctx->for_cap * sizeof(ForFrame));
    }
}

static void ensure_while_capacity(ExecutionContext *ctx)
{
    if (ctx->while_sp >= ctx->while_cap)
    {
        ctx->while_cap = (ctx->while_cap == 0) ? 16 : ctx->while_cap * 2;
        ctx->while_stack = xrealloc(ctx->while_stack, ctx->while_cap * sizeof(WhileFrame));
    }
}

static int expr_is_string(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0;
    }

    if (expr->type == EXPR_STRING)
    {
        return 1;
    }

    if (expr->type == EXPR_VAR || expr->type == EXPR_ARRAY || expr->type == EXPR_FUNC_CALL)
    {
        if (expr->var_name && expr->var_name[0] != '\0')
        {
            size_t len = strlen(expr->var_name);
            if (expr->var_name[len - 1] == '$')
            {
                return 1;
            }
        }

        if (expr->type == EXPR_VAR || expr->type == EXPR_ARRAY)
        {
            if (expr->var_name && runtime_get_variable_type(state, expr->var_name) == VAR_STRING)
            {
                return 1;
            }
        }
    }

    if (expr->type == EXPR_BINARY_OP && expr->op == OP_CONCAT)
    {
        /* String concatenation produces a string */
        return 1;
    }

    return 0;
}

/* Execute PRINT statement */
static int execute_print_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        if (stmt && stmt->file_handle > 0)
        {
            FILE *fp = runtime_get_file(ctx->runtime, stmt->file_handle);
            if (fp)
            {
                fprintf(fp, "\n");
                fflush(fp);
            }
        }
        else
        {
            termio_write("\n");
            termio_present();
            runtime_set_output_pending(ctx->runtime, 0);
        }
        return 0;
    }

    FILE *out = NULL;
    if (stmt->file_handle > 0)
    {
        out = runtime_get_file(ctx->runtime, stmt->file_handle);
    }
    int console_output = (out == NULL);
    int output_pending = 0;
    int output_col = 0;
    if (console_output)
    {
        output_pending = runtime_get_output_pending(ctx->runtime);
        if (output_pending)
        {
            output_col = runtime_get_output_col(ctx->runtime);
        }
    }

    /* Check if last expression is a separator (suppress newline) */
    int trailing_separator = 0;
    if (stmt->num_exprs > 0)
    {
        ASTExpr *last = stmt->exprs[stmt->num_exprs - 1];
        if (last->type == EXPR_PRINT_SEP && last->str_value != NULL &&
            (strcmp(last->str_value, ";") == 0 || strcmp(last->str_value, ",") == 0))
        {
            trailing_separator = 1;
        }
    }

    for (int i = 0; i < stmt->num_exprs; i++)
    {
        ASTExpr *expr = stmt->exprs[i];

        /* Check for special print directives */
        if (expr->type == EXPR_PRINT_SEP && expr->str_value != NULL)
        {
            /* Check for semicolon (suppress newline) */
            if (strcmp(expr->str_value, ";") == 0)
            {
                /* Semicolon concatenates without extra spacing */
                continue;
            }
            /* Check for comma (tab spacing) */
            else if (strcmp(expr->str_value, ",") == 0)
            {
                const int zone_width = 14;
                const int line_width = 80;
                int next_zone = ((output_col / zone_width) + 1) * zone_width;
                if (next_zone >= line_width)
                {
                    if (out)
                    {
                        fputc('\n', out);
                        fflush(out);
                    }
                    else
                    {
                        termio_write("\n");
                        termio_present();
                    }
                    output_col = 0;
                    output_pending = 0;
                }
                else
                {
                    int spaces = next_zone - output_col;
                    for (int s = 0; s < spaces; s++)
                    {
                        if (out)
                            fputc(' ', out);
                        else
                            termio_write_char(' ');
                    }
                    output_col = next_zone;
                }
                continue;
            }
        }

        /* Handle TAB(n) function */
        if (expr->type == EXPR_TAB)
        {
            if (expr->num_children > 0 && expr->children[0])
            {
                double tab_pos = eval_numeric_expr(ctx->runtime, expr->children[0]);
                int target_col = (int)tab_pos;
                if (target_col < 0)
                    target_col = 0;

                /* Only output spaces if we need to move forward */
                if (target_col > output_col)
                {
                    for (int s = output_col; s < target_col; s++)
                    {
                        if (out)
                            fputc(' ', out);
                        else
                            termio_write_char(' ');
                    }
                    output_col = target_col;
                }
            }
            continue;
        }

        /* Evaluate and print expression */
        if (expr_is_string(ctx->runtime, expr))
        {
            char *str_val = eval_string_expr(ctx->runtime, expr);
            if (out)
                fprintf(out, "%s", str_val);
            else
                termio_write(str_val);
            output_col += (int)strlen(str_val);
            free(str_val);
        }
        else
        {
            double num_val = eval_numeric_expr(ctx->runtime, expr);
            char buf[64];
            if (fabs(num_val) < 1e-10 && num_val != 0.0)
                snprintf(buf, sizeof(buf), "%.9e", num_val);
            else
                snprintf(buf, sizeof(buf), "%.15g", num_val);

            if (out)
                fprintf(out, "%s", buf);
            else
                termio_write(buf);
            output_col += (int)strlen(buf);
        }

        /* Add space after item if next exists and isn't a separator */
        if (i + 1 < stmt->num_exprs)
        {
            ASTExpr *next = stmt->exprs[i + 1];
            int next_is_separator = (next->type == EXPR_PRINT_SEP && next->str_value != NULL &&
                                     (strcmp(next->str_value, ";") == 0 || strcmp(next->str_value, ",") == 0));
            if (!next_is_separator)
            {
                if (out)
                    fprintf(out, " ");
                else
                    termio_write_char(' ');
                output_col += 1;
            }
        }
    }

    if (!trailing_separator)
    {
        if (out)
        {
            fprintf(out, "\n");
            fflush(out);
        }
        else
        {
            termio_write("\n");
            termio_present();
            output_col = 0;
            output_pending = 0;
        }
    }
    else if (console_output)
    {
        output_pending = 1;
    }

    if (console_output)
    {
        runtime_set_output_pending(ctx->runtime, output_pending);
        runtime_set_output_col(ctx->runtime, output_col);
    }

    return 0;
}

static int execute_print_at_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        return 0;
    }

    double pos = eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    int ipos = (int)pos;
    if (ipos < 0)
        ipos = 0;
    int row = (ipos / 80) + 1;
    int col = (ipos % 80) + 1;

    /* Set cursor position using termio function */
    termio_set_cursor(row, col);

    ASTExpr *expr = stmt->exprs[1];
    if (expr_is_string(ctx->runtime, expr))
    {
        char *str_val = eval_string_expr(ctx->runtime, expr);
        termio_write(str_val);
        free(str_val);
    }
    else
    {
        double num_val = eval_numeric_expr(ctx->runtime, expr);
        char buf[64];
        if (fabs(num_val) < 1e-10 && num_val != 0.0)
            snprintf(buf, sizeof(buf), "%.9e", num_val);
        else
            snprintf(buf, sizeof(buf), "%.15g", num_val);
        termio_write(buf);
    }

    /* Force display update for SDL */
    termio_present();

    return 0;
}

static int execute_print_using_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        return 0;
    }

    /* First expression is the format string */
    ASTExpr *format_expr = stmt->exprs[0];
    char *format_str = eval_string_expr(ctx->runtime, format_expr);
    if (!format_str)
    {
        return 0;
    }

    /* Second expression is the value to format */
    ASTExpr *value_expr = stmt->exprs[1];
    double value = eval_numeric_expr(ctx->runtime, value_expr);

    /* Apply the format string */
    char formatted[256];

    /* Check if format contains $ for currency */
    if (strstr(format_str, "$") != NULL)
    {
        snprintf(formatted, sizeof(formatted), "$%.2f", value);
    }
    /* Check for # patterns for number formatting */
    else if (strstr(format_str, "#") != NULL)
    {
        /* Count decimal places after . in format string */
        const char *dot = strchr(format_str, '.');
        int decimal_places = 2; /* default */
        if (dot)
        {
            decimal_places = 0;
            const char *p = dot + 1;
            while (*p == '#' || *p == '.')
            {
                if (*p == '#')
                    decimal_places++;
                p++;
            }
        }
        snprintf(formatted, sizeof(formatted), "%.*f", decimal_places, value);
    }
    else
    {
        /* Generic number format */
        snprintf(formatted, sizeof(formatted), "%.15g", value);
    }

    termio_write(formatted);
    termio_write("\n");
    termio_present();

    free(format_str);
    return 0;
}

static int execute_line_input_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    ASTExpr *expr = stmt->exprs[0];
    if (!expr || !expr->var_name)
    {
        return 0;
    }

    FILE *fp = (stmt->file_handle > 0) ? runtime_get_file(ctx->runtime, stmt->file_handle) : stdin;
    if (!fp)
    {
        return -1;
    }

    char line[1024];
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        return 0;
    }

    size_t len = strlen(line);
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
    {
        line[len - 1] = '\0';
    }

    runtime_set_string_variable(ctx->runtime, expr->var_name, line);
    return 0;
}

/* Execute INPUT statement */
static int execute_input_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    /* File INPUT #n */
    if (stmt->file_handle > 0)
    {
        FILE *fp = runtime_get_file(ctx->runtime, stmt->file_handle);
        if (!fp)
        {
            return -1;
        }

        char line[1024];
        if (fgets(line, sizeof(line), fp) == NULL)
        {
            return 0;
        }

        /* Strip newline */
        size_t len = strlen(line);
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[len - 1] = '\0';
        }

        /* Parse CSV fields */
        char *p = line;
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            ASTExpr *expr = stmt->exprs[i];
            if (!expr || !expr->var_name)
            {
                continue;
            }

            /* Skip leading spaces */
            while (*p == ' ' || *p == '\t')
                p++;

            char field[512];
            int fi = 0;
            int quoted = 0;
            if (*p == '"')
            {
                quoted = 1;
                p++;
                while (*p && *p != '"' && fi < 511)
                {
                    field[fi++] = *p++;
                }
                if (*p == '"')
                    p++;
            }
            else
            {
                while (*p && *p != ',' && fi < 511)
                {
                    field[fi++] = *p++;
                }
            }
            field[fi] = '\0';

            /* Skip comma */
            if (*p == ',')
                p++;

            VarType var_type = runtime_get_variable_type(ctx->runtime, expr->var_name);
            if (var_type == VAR_STRING)
            {
                if (quoted)
                {
                    char quoted_field[520];
                    snprintf(quoted_field, sizeof(quoted_field), "\"%s\"", field);
                    runtime_set_string_variable(ctx->runtime, expr->var_name, quoted_field);
                }
                else
                {
                    runtime_set_string_variable(ctx->runtime, expr->var_name, field);
                }
            }
            else
            {
                double num_val = strtod(field, NULL);
                runtime_set_variable(ctx->runtime, expr->var_name, num_val);
            }
        }

        return 0;
    }

    char input_buffer[1024];

    /* Check if first expression is a prompt string */
    int var_start = 0;
    const char *prompt = "? ";

    if (stmt->num_exprs > 0 && stmt->exprs[0] && stmt->exprs[0]->type == EXPR_STRING)
    {
        prompt = stmt->exprs[0]->str_value;
        var_start = 1;
    }

    for (int i = var_start; i < stmt->num_exprs; i++)
    {
        /* Check for interrupt before each input prompt */
        if (g_interrupt_flag && *g_interrupt_flag)
        {
            *g_interrupt_flag = 0;
            return -1;
        }

        ASTExpr *expr = stmt->exprs[i];

        if (expr == NULL || expr->var_name == NULL)
        {
            continue;
        }

        memset(input_buffer, 0, sizeof(input_buffer));
        termio_write(prompt);
        termio_present();

        int len = termio_readline(input_buffer, sizeof(input_buffer));

        /* Check for interrupt after input */
        if (g_interrupt_flag && *g_interrupt_flag)
        {
            *g_interrupt_flag = 0;
            termio_write("\n");
            return -1;
        }

        if (len < 0)
        {
            /* Fallback to stdin if termio fails */
            termio_write("\n");
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL)
            {
                return -1;
            }
            size_t slen = strlen(input_buffer);
            if (slen > 0 && input_buffer[slen - 1] == '\n')
            {
                input_buffer[slen - 1] = '\0';
            }
        }
        else
        {
            /* Ensure null termination from termio_readline */
            input_buffer[len] = '\0';
            /* termio_readline already handles echo and newline */
        }

        /* Determine variable type and set accordingly */
        VarType var_type = runtime_get_variable_type(ctx->runtime, expr->var_name);

        if (var_type == VAR_STRING)
        {
            runtime_set_string_variable(ctx->runtime, expr->var_name, input_buffer);
        }
        else
        {
            /* Parse as number */
            char *endptr;
            double num_val = strtod(input_buffer, &endptr);
            /* Check if conversion was successful - endptr should be at null terminator */
            if (input_buffer[0] == '\0' || *endptr != '\0')
            {
                /* Empty input or invalid number - default to 0 */
                num_val = 0.0;
            }
            runtime_set_variable(ctx->runtime, expr->var_name, num_val);
        }
    }

    return 0;
}

/* Execute LET statement (variable assignment) */
static int execute_let_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        return 0;
    }

    ASTExpr *lhs = stmt->exprs[0];
    ASTExpr *rhs = stmt->exprs[1];

    if (lhs == NULL || rhs == NULL)
    {
        return 0;
    }

    /* Array assignment: lhs is EXPR_ARRAY with children as indices */
    if (lhs->type == EXPR_ARRAY)
    {
        int num_indices = lhs->num_children;
        int *indices = xmalloc(sizeof(int) * (num_indices > 0 ? num_indices : 1));

        for (int i = 0; i < num_indices; i++)
        {
            indices[i] = (int)eval_numeric_expr(ctx->runtime, lhs->children[i]);
        }

        VarType vtype = runtime_get_variable_type(ctx->runtime, lhs->var_name);
        if (vtype == VAR_STRING)
        {
            char *str_val = eval_string_expr(ctx->runtime, rhs);
            runtime_set_string_array_element(ctx->runtime, lhs->var_name, indices, num_indices, str_val);
            free(str_val);
        }
        else
        {
            double value = eval_numeric_expr(ctx->runtime, rhs);
            runtime_set_array_element(ctx->runtime, lhs->var_name, indices, num_indices, value);
        }

        free(indices);
    }
    else
    {
        /* Simple variable assignment */
        const char *var_name = lhs->var_name;
        VarType var_type = runtime_get_variable_type(ctx->runtime, var_name);

        if (var_type == VAR_STRING)
        {
            char *str_val = eval_string_expr(ctx->runtime, rhs);

            /* Check for runtime errors (e.g., type mismatch) */
            int err = runtime_get_error(ctx->runtime);
            if (err != 0)
            {
                free(str_val);
                return -err; /* Return negated error code */
            }

            runtime_set_string_variable(ctx->runtime, var_name, str_val);
            free(str_val);
        }
        else
        {
            double num_val = eval_numeric_expr(ctx->runtime, rhs);
            runtime_set_variable(ctx->runtime, var_name, num_val);
        }
    }

    return 0;
}

/* Execute IF statement */
static int execute_if_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    if (stmt->num_exprs == 0)
    {
        return 0;
    }

    /* Evaluate condition */
    double condition = eval_condition(ctx->runtime, stmt->exprs[0]);

    if (condition != 0)
    {
        /* Condition is true */
        if (stmt->body != NULL)
        {
            return execute_stmt_internal(ctx, stmt->body);
        }
    }
    else
    {
        if (stmt->else_body != NULL)
        {
            return execute_stmt_internal(ctx, stmt->else_body);
        }
    }

    return 0;
}

static int execute_on_goto_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        return 0;
    }

    int index = (int)eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    if (index <= 0)
    {
        return 0;
    }

    int target_idx = index; /* exprs[1] is first target */
    if (target_idx >= stmt->num_exprs)
    {
        return 0;
    }

    ASTExpr *line_expr = stmt->exprs[target_idx];
    if (!line_expr)
    {
        return 0;
    }

    int line_num = (int)line_expr->num_value;
    int target_line_index = find_program_line(ctx->program, line_num);
    if (target_line_index < 0)
    {
        return -1;
    }

    if (stmt->mode == 1)
    {
        int return_line = (ctx->current_line_index + 1 < ctx->program->num_lines)
                              ? ctx->program->lines[ctx->current_line_index + 1]->line_number
                              : -1;
        runtime_push_call(ctx->runtime, return_line);
    }

    ctx->next_line_index = target_line_index;
    return 0;
}

/* Execute FOR statement */
static int execute_for_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 3)
    {
        return 0;
    }

    ASTExpr *var_expr = stmt->exprs[0];
    if (var_expr == NULL || var_expr->var_name == NULL)
    {
        return 0;
    }

    /* Get loop variable name */
    const char *loop_var = var_expr->var_name;

    /* Evaluate start, end, and optional step */
    double start = eval_numeric_expr(ctx->runtime, stmt->exprs[1]);
    double end = eval_numeric_expr(ctx->runtime, stmt->exprs[2]);
    double step = (stmt->num_exprs > 3) ? eval_numeric_expr(ctx->runtime, stmt->exprs[3]) : 1.0;

    if (step == 0.0)
    {
        /* Invalid step - avoid infinite loop */
        return -1;
    }

    /* Set loop variable to start value */
    runtime_set_variable(ctx->runtime, loop_var, start);

    /* Find the NEXT statement for this FOR loop */
    int next_line_index = -1;
    int nesting_level = 0;
    ASTStmt *body_start = NULL;
    ASTStmt *after_next = NULL;

    /* First, check for NEXT in chained statements on the same line (after this FOR) */
    ASTStmt *current_stmt = stmt->next;
    while (current_stmt != NULL)
    {
        if (current_stmt->type == STMT_FOR)
        {
            nesting_level++;
        }
        else if (current_stmt->type == STMT_NEXT)
        {
            int count = (current_stmt->num_exprs > 0) ? current_stmt->num_exprs : 1;
            if (nesting_level < count)
            {
                next_line_index = ctx->current_line_index;
                body_start = stmt->next;
                after_next = current_stmt->next;
                break;
            }
            nesting_level -= count;
        }
        current_stmt = current_stmt->next;
    }

    /* If not found on same line, search subsequent lines */
    if (next_line_index < 0)
    {
        for (int i = ctx->current_line_index + 1; i < ctx->program->num_lines; i++)
        {
            ASTStmt *line_stmt = ctx->program->lines[i]->stmt;

            if (line_stmt->type == STMT_FOR)
            {
                nesting_level++;
            }
            else if (line_stmt->type == STMT_NEXT)
            {
                int count = (line_stmt->num_exprs > 0) ? line_stmt->num_exprs : 1;
                if (nesting_level < count)
                {
                    next_line_index = i;
                    break;
                }
                nesting_level -= count;
            }
        }
    }

    if (next_line_index < 0)
    {
        /* NEXT not found */
        return -1;
    }

    ensure_for_capacity(ctx);
    ForFrame *frame = &ctx->for_stack[ctx->for_sp++];
    frame->var_name = xstrdup(loop_var);
    frame->end = end;
    frame->step = step;
    frame->for_line_index = ctx->current_line_index;
    frame->next_line_index = next_line_index;
    frame->body_start = body_start;
    frame->after_next = after_next;

    /* Continue to next line after FOR */
    ctx->next_line_index = ctx->current_line_index + 1;
    return 0;
}

static int execute_next_for_var(ExecutionContext *ctx, const char *name)
{
    if (ctx->for_sp <= 0)
    {
        return 0;
    }

    ForFrame *frame = NULL;
    int frame_index = ctx->for_sp - 1;

    if (name)
    {
        for (int i = ctx->for_sp - 1; i >= 0; i--)
        {
            if (ctx->for_stack[i].var_name && strcmp(ctx->for_stack[i].var_name, name) == 0)
            {
                frame_index = i;
                break;
            }
        }
    }

    frame = &ctx->for_stack[frame_index];

    /* Check for interrupt (Ctrl-C) - exit FOR loop immediately without consuming the flag */
    if (g_interrupt_flag && *g_interrupt_flag)
    {
        /* Clean up FOR frame and exit - DON'T reset the flag here */
        free(frame->var_name);
        if (frame_index != ctx->for_sp - 1)
        {
            memmove(&ctx->for_stack[frame_index], &ctx->for_stack[frame_index + 1],
                    (ctx->for_sp - frame_index - 1) * sizeof(ForFrame));
        }
        ctx->for_sp--;
        ctx->next_line_index = frame->next_line_index + 1;
        return 0;
    }

    /* Process SDL events periodically to keep UI responsive */
    static int event_counter = 0;
    if (++event_counter >= 10000)
    {
        event_counter = 0;
        executor_process_events();
    }

    double loop_value = runtime_get_variable(ctx->runtime, frame->var_name);
    loop_value += frame->step;
    runtime_set_variable(ctx->runtime, frame->var_name, loop_value);

    if ((frame->step > 0 && loop_value <= frame->end) || (frame->step < 0 && loop_value >= frame->end))
    {
        if (frame->next_line_index == frame->for_line_index && frame->body_start != NULL)
        {
            ctx->next_line_index = frame->for_line_index;
            ctx->next_stmt_override = frame->body_start;
        }
        else
        {
            ctx->next_line_index = frame->for_line_index + 1;
        }
        return 1;
    }
    else
    {
        free(frame->var_name);
        if (frame_index != ctx->for_sp - 1)
        {
            memmove(&ctx->for_stack[frame_index], &ctx->for_stack[frame_index + 1],
                    (ctx->for_sp - frame_index - 1) * sizeof(ForFrame));
        }
        ctx->for_sp--;
        if (frame->next_line_index == frame->for_line_index && frame->after_next != NULL)
        {
            ctx->next_line_index = frame->for_line_index;
            ctx->next_stmt_override = frame->after_next;
        }
        else
        {
            ctx->next_line_index = frame->next_line_index + 1;
        }
    }
    return 0;
    return 0;
}

/* Execute WHILE statement */
static int execute_while_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 1)
    {
        return 0;
    }

    ASTExpr *condition = stmt->exprs[0];
    if (condition == NULL)
    {
        return 0;
    }

    /* Evaluate condition */
    int cond_value = eval_condition(ctx->runtime, condition);

    /* If we're re-entering the same WHILE via WEND, reuse the existing frame */
    if (ctx->while_sp > 0)
    {
        WhileFrame *top = &ctx->while_stack[ctx->while_sp - 1];
        if (top->while_line_index == ctx->current_line_index)
        {
            top->condition = condition;
            if (!cond_value)
            {
                /* Exit loop: pop frame and skip past matching WEND */
                int wend_line_index = top->wend_line_index;
                ctx->while_sp--;
                if (wend_line_index >= 0)
                {
                    ctx->next_line_index = wend_line_index + 1;
                }
                else
                {
                    return -BASIC_ERR_NEXT_WITHOUT_FOR;
                }
            }
            else
            {
                /* Continue into loop body */
                ctx->next_line_index = ctx->current_line_index + 1;
            }

            if (getenv("AST_DEBUG"))
            {
                fprintf(stderr, "[AST] WHILE reuse line_index=%d sp=%d\n",
                        ctx->current_line_index, ctx->while_sp);
            }

            return 0;
        }
    }

    if (!cond_value)
    {
        /* Condition is false - skip to line after WEND */
        int wend_line_index = -1;
        int nesting_level = 0;

        /* Search for matching WEND */
        for (int i = ctx->current_line_index + 1; i < ctx->program->num_lines; i++)
        {
            ASTStmt *line_stmt = ctx->program->lines[i]->stmt;

            if (line_stmt->type == STMT_WHILE)
            {
                nesting_level++;
            }
            else if (line_stmt->type == STMT_WEND)
            {
                if (nesting_level == 0)
                {
                    wend_line_index = i;
                    break;
                }
                nesting_level--;
            }
        }

        if (wend_line_index >= 0)
        {
            ctx->next_line_index = wend_line_index + 1;
        }
        else
        {
            /* WEND not found */
            return -BASIC_ERR_NEXT_WITHOUT_FOR;
        }
    }
    else
    {
        /* Condition is true - push frame and continue to next line */
        ensure_while_capacity(ctx);
        WhileFrame *frame = &ctx->while_stack[ctx->while_sp++];
        frame->condition = condition;
        frame->while_line_index = ctx->current_line_index;

        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] WHILE push line_index=%d sp=%d\n",
                    frame->while_line_index, ctx->while_sp);
        }

        /* Find matching WEND */
        int wend_line_index = -1;
        int nesting_level = 0;
        for (int i = ctx->current_line_index + 1; i < ctx->program->num_lines; i++)
        {
            ASTStmt *line_stmt = ctx->program->lines[i]->stmt;

            if (line_stmt->type == STMT_WHILE)
            {
                nesting_level++;
            }
            else if (line_stmt->type == STMT_WEND)
            {
                if (nesting_level == 0)
                {
                    wend_line_index = i;
                    break;
                }
                nesting_level--;
            }
        }

        if (wend_line_index >= 0)
        {
            frame->wend_line_index = wend_line_index;
        }
        else
        {
            ctx->while_sp--;
            return -BASIC_ERR_NEXT_WITHOUT_FOR;
        }

        /* Continue to next line */
        ctx->next_line_index = ctx->current_line_index + 1;
    }

    return 0;
}

/* Execute WEND statement */
static int execute_wend_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)stmt;

    if (ctx->while_sp <= 0)
    {
        return -BASIC_ERR_NEXT_WITHOUT_FOR;
    }

    WhileFrame *frame = &ctx->while_stack[ctx->while_sp - 1];

    /* Re-evaluate the WHILE condition */
    int cond_value = eval_condition(ctx->runtime, frame->condition);

    if (cond_value)
    {
        /* Condition is still true - jump back to WHILE */
        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] WEND cond=1 line_index=%d -> while_index=%d sp=%d\n",
                    ctx->current_line_index, frame->while_line_index, ctx->while_sp);
        }
        ctx->next_line_index = frame->while_line_index;
    }
    else
    {
        /* Condition is false - pop frame and continue */
        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] WEND cond=0 line_index=%d pop sp=%d\n",
                    ctx->current_line_index, ctx->while_sp);
        }
        ctx->while_sp--;
        ctx->next_line_index = ctx->current_line_index + 1;
    }

    return 0;
}

static int execute_do_loop_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    RuntimeState *runtime = ctx->runtime;

    /* Check if this is a DO statement (is_loop_end=0) or LOOP statement (is_loop_end=1) */
    if (stmt->is_loop_end == 0) /* This is a DO statement */
    {
        /* Push new frame onto do_loop_stack */
        runtime_push_do_loop(runtime, ctx->current_line_index, stmt->data.condition_type,
                             stmt->num_exprs > 0 ? stmt->exprs[0] : NULL);

        /* For DO WHILE (pre-test), evaluate condition first */
        if (stmt->data.condition_type == 1) /* pre-test WHILE */
        {
            ASTExpr *condition = stmt->num_exprs > 0 ? stmt->exprs[0] : NULL;
            int cond_value = eval_condition(runtime, condition);
            if (!cond_value)
            {
                /* Condition is false - skip to line after LOOP */
                int loop_line_index = -1;
                int nesting_level = 0;

                /* Search for matching LOOP */
                for (int i = ctx->current_line_index + 1; i < ctx->program->num_lines; i++)
                {
                    ASTStmt *line_stmt = ctx->program->lines[i]->stmt;
                    if (line_stmt->type == STMT_DO_LOOP && line_stmt->is_loop_end == 0)
                    {
                        nesting_level++;
                    }
                    else if (line_stmt->type == STMT_DO_LOOP && line_stmt->is_loop_end == 1)
                    {
                        if (nesting_level == 0)
                        {
                            loop_line_index = i;
                            break;
                        }
                        nesting_level--;
                    }
                }

                runtime_pop_do_loop(runtime, NULL); /* Pop frame */
                if (loop_line_index >= 0)
                {
                    ctx->next_line_index = loop_line_index + 1;
                }
                else
                {
                    return -BASIC_ERR_SYNTAX_ERROR; /* No matching LOOP found */
                }
            }
            else
            {
                /* Condition is true - enter loop body */
                ctx->next_line_index = ctx->current_line_index + 1;
            }
        }
        else
        {
            /* Infinite DO or DO LOOP UNTIL/WHILE (post-test) - enter loop body */
            ctx->next_line_index = ctx->current_line_index + 1;
        }
    }
    else /* This is a LOOP statement (is_loop_end=1) */
    {
        if (runtime_get_do_loop_depth(runtime) <= 0)
        {
            return -BASIC_ERR_SYNTAX_ERROR; /* LOOP without DO */
        }

        int do_line_index = runtime_get_current_do_line(runtime);
        runtime_set_current_loop_line(runtime, ctx->current_line_index);

        /* Use LOOP statement's condition_type if present, otherwise use frame's */
        int condition_type = stmt->data.condition_type;
        if (condition_type == 0)
        {
            /* LOOP statement has no condition, check the frame's condition_type */
            condition_type = runtime_get_current_condition_type(runtime);
        }

        ASTExpr *stored_condition = (ASTExpr *)runtime_get_current_condition(runtime);
        ASTExpr *loop_condition = stmt->num_exprs > 0 ? stmt->exprs[0] : stored_condition;

        int should_continue = 0;

        if (condition_type == 0) /* Plain LOOP (infinite) */
        {
            should_continue = 1;
        }
        else if (condition_type == 2) /* LOOP WHILE (post-test while) */
        {
            should_continue = eval_condition(runtime, loop_condition);
        }
        else if (condition_type == 3) /* LOOP UNTIL (post-test until) */
        {
            int cond_value = eval_condition(runtime, loop_condition);
            should_continue = !cond_value; /* Continue if condition is false */
        }
        else if (condition_type == 1) /* DO WHILE already tested pre-test */
        {
            should_continue = eval_condition(runtime, stored_condition);
        }

        if (should_continue)
        {
            /* Jump back to first statement after DO (loop body), not to DO itself */
            /* do_line_index points to the line with DO statement */
            /* We want ctx->next_line_index to be do_line_index + 1 */
            ctx->next_line_index = do_line_index + 1;
        }
        else
        {
            /* Exit loop */
            runtime_pop_do_loop(runtime, NULL);
            ctx->next_line_index = ctx->current_line_index + 1;
        }
    }

    return 0;
}

static int execute_exit_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)stmt;

    RuntimeState *runtime = ctx->runtime;
    if (runtime_get_do_loop_depth(runtime) <= 0)
    {
        /* EXIT outside of DO loop - terminate program */
        return 1; /* Signal end of program */
    }

    /* Pop the DO..LOOP frame and jump to after the matching LOOP */
    int loop_line_index = -1;
    runtime_pop_do_loop(runtime, &loop_line_index);

    if (loop_line_index >= 0)
    {
        ctx->next_line_index = loop_line_index + 1;
    }
    else
    {
        /* Loop_line_index not yet set? Search for matching LOOP */
        int nesting_level = 1; /* We're already inside one DO loop */

        for (int i = ctx->current_line_index; i < ctx->program->num_lines; i++)
        {
            ASTStmt *line_stmt = ctx->program->lines[i]->stmt;
            if (line_stmt->type == STMT_DO_LOOP && line_stmt->is_loop_end == 0)
            {
                nesting_level++;
            }
            else if (line_stmt->type == STMT_DO_LOOP && line_stmt->is_loop_end == 1)
            {
                nesting_level--;
                if (nesting_level == 0)
                {
                    ctx->next_line_index = i + 1;
                    return 0;
                }
            }
        }
    }

    return 0;
}

static int execute_next_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (!stmt || stmt->num_exprs == 0)
    {
        int continued = execute_next_for_var(ctx, NULL);
        if (continued && stmt && stmt->next != NULL && ctx->next_stmt_override == NULL &&
            ctx->next_line_index != ctx->current_line_index)
        {
            ctx->skip_chained = 1;
        }
        return 0;
    }

    for (int i = 0; i < stmt->num_exprs; i++)
    {
        ASTExpr *var = stmt->exprs[i];
        const char *name = var ? var->var_name : NULL;
        if (execute_next_for_var(ctx, name))
        {
            if (stmt->next != NULL && ctx->next_stmt_override == NULL &&
                ctx->next_line_index != ctx->current_line_index)
            {
                ctx->skip_chained = 1;
            }
            break;
        }
    }

    return 0;
}

/* Execute GOTO statement */
static int execute_goto_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->target_line <= 0)
    {
        return 0;
    }

    int target_index = find_program_line(ctx->program, stmt->target_line);

    if (target_index < 0)
    {
        /* Line not found - undefined line */
        return -BASIC_ERR_UNDEFINED_LINE;
    }

    ctx->next_line_index = target_index;

    return 0;
}

/* Execute GOSUB statement */
static int execute_gosub_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->target_line <= 0)
    {
        return 0;
    }

    int target_index = find_program_line(ctx->program, stmt->target_line);

    if (target_index < 0)
    {
        /* Line not found */
        return -BASIC_ERR_UNDEFINED_LINE;
    }

    /* Push return line onto call stack */
    int return_line = (ctx->current_line_index + 1 < ctx->program->num_lines)
                          ? ctx->program->lines[ctx->current_line_index + 1]->line_number
                          : -1;

    runtime_push_call(ctx->runtime, return_line);

    /* Jump to subroutine */
    ctx->next_line_index = target_index;

    return 0;
}
/* Execute RETURN statement */
static int execute_return_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    /* RETURN only valid inside procedures (no GOSUB in this interpreter) */
    if (!ctx->in_procedure)
    {
        runtime_set_error(ctx->runtime, 3, 0); /* RETURN without procedure */
        return -3;
    }

    /* Evaluate return value if present */
    if (stmt && stmt->num_exprs > 0)
    {
        double ret_val = ast_eval_expr(stmt->exprs[0]);
        ctx->proc_return_value = ret_val;
    }
    else
    {
        ctx->proc_return_value = 0.0;
    }

    /* Set flag to exit procedure */
    ctx->proc_return_flag = 1;
    return 0;
}

/* Helper: Find procedure definition by name */
static ASTStmt *find_procedure_def(Program *prog, const char *proc_name)
{
    if (!prog || !proc_name)
        return NULL;

    for (int i = 0; i < prog->num_lines; i++)
    {
        ASTStmt *stmt = prog->lines[i]->stmt;
        if (stmt && stmt->type == STMT_PROCEDURE_DEF && stmt->var_name)
        {
            if (strcasecmp(stmt->var_name, proc_name) == 0)
            {
                return stmt;
            }
        }
    }
    return NULL;
}

/* Execute PROCEDURE CALL statement */
static int execute_procedure_call_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (!stmt || !stmt->var_name)
        return 0;

    /* Find procedure definition */
    ASTStmt *proc_def = find_procedure_def(ctx->program, stmt->var_name);
    if (!proc_def)
    {
        /* Procedure not found - treat as error */
        runtime_set_error(ctx->runtime, 251, 0); /* Illegal function call */
        return -251;
    }

    /* Create and push a new scope for this procedure call */
    ProcedureScope *new_scope = proc_scope_create();
    proc_scope_push(ctx, new_scope);

    /* Save original values of parameter names (to restore after) */
    if (proc_def->parameters)
    {
        for (int i = 0; i < proc_def->parameters->num_params; i++)
        {
            ASTParameter *param = proc_def->parameters->params[i];
            if (param && param->name)
            {
                /* Save the original value if variable exists */
                double original_value = 0.0;
                if (runtime_has_variable(ctx->runtime, param->name))
                {
                    original_value = runtime_get_variable(ctx->runtime, param->name);
                }
                proc_scope_save_var(ctx, param->name, original_value);
            }
        }
    }

    /* Bind arguments to parameters as local variables */
    if (proc_def->parameters)
    {
        for (int i = 0; i < proc_def->parameters->num_params && i < stmt->num_call_args; i++)
        {
            ASTParameter *param = proc_def->parameters->params[i];
            ASTExpr *arg = stmt->call_args[i];

            if (arg && param && param->name)
            {
                /* Evaluate argument in caller's scope */
                double arg_value = ast_eval_expr(arg);

                /* Store argument as local variable in procedure scope */
                runtime_set_variable(ctx->runtime, param->name, arg_value);
            }
        }
    }

    /* Mark that we're in a procedure */
    int saved_in_procedure = ctx->in_procedure;
    ctx->in_procedure = 1;
    int saved_proc_return_flag = ctx->proc_return_flag;
    ctx->proc_return_flag = 0;

    /* Execute procedure body */
    int result = 0;
    if (proc_def->body)
    {
        result = execute_stmt_internal(ctx, proc_def->body);
    }

    /* Save return value before scope restoration */
    double return_value = ctx->proc_return_value;

    /* Restore procedure context */
    ctx->proc_return_flag = saved_proc_return_flag;
    ctx->in_procedure = saved_in_procedure;

    /* Restore global variables shadowed by parameters */
    ProcedureScope *scope = proc_scope_pop(ctx);
    if (scope)
    {
        for (int i = 0; i < scope->num_vars; i++)
        {
            /* Only restore if the variable existed before the call */
            if (scope->var_values[i] != 0.0 || runtime_has_variable(ctx->runtime, scope->var_names[i]))
            {
                runtime_set_variable(ctx->runtime, scope->var_names[i], scope->var_values[i]);
            }
            else
            {
                /* Variable was created in procedure, remove it from global scope */
                runtime_delete_variable(ctx->runtime, scope->var_names[i]);
            }
        }
        proc_scope_free(scope);
    }

    /* Store return value in 'result' variable for caller to access */
    runtime_set_variable(ctx->runtime, "result", return_value);
    ctx->proc_return_value = return_value;

    return result;
}

/* Execute DIM statement (array dimension) */
static int execute_dim_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    for (int a = 0; a < stmt->num_exprs; a++)
    {
        ASTExpr *array = stmt->exprs[a];
        if (array == NULL || array->var_name == NULL)
        {
            continue;
        }

        int num_dims = array->num_children;
        if (num_dims <= 0)
        {
            continue;
        }

        /* Evaluate dimension sizes */
        int *dimensions = xmalloc(sizeof(int) * num_dims);

        for (int i = 0; i < num_dims; i++)
        {
            dimensions[i] = (int)eval_numeric_expr(ctx->runtime, array->children[i]);

            if (dimensions[i] <= 0)
            {
                free(dimensions);
                return -1;
            }
        }

        /* Allocate array */
        runtime_dim_array(ctx->runtime, array->var_name, dimensions, num_dims);

        free(dimensions);
    }

    return 0;
}

/* Execute READ statement */
static int execute_read_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    for (int i = 0; i < stmt->num_exprs; i++)
    {
        ASTExpr *var = stmt->exprs[i];
        if (!var)
        {
            continue;
        }

        VarType dtype = VAR_DOUBLE;
        double num_val = 0.0;
        char *str_val = NULL;
        if (!runtime_data_read(ctx->runtime, &dtype, &num_val, &str_val))
        {
            return -BASIC_ERR_OUT_OF_DATA;
        }

        if (var->type == EXPR_ARRAY)
        {
            int num_indices = var->num_children;
            int *indices = xmalloc(sizeof(int) * (num_indices > 0 ? num_indices : 1));
            for (int j = 0; j < num_indices; j++)
            {
                indices[j] = (int)eval_numeric_expr(ctx->runtime, var->children[j]);
            }

            VarType vtype = runtime_get_variable_type(ctx->runtime, var->var_name);
            if (vtype == VAR_STRING)
            {
                const char *src = (dtype == VAR_STRING && str_val) ? str_val : "";
                runtime_set_string_array_element(ctx->runtime, var->var_name, indices, num_indices, src);
            }
            else
            {
                double value = (dtype == VAR_STRING && str_val) ? strtod(str_val, NULL) : num_val;
                runtime_set_array_element(ctx->runtime, var->var_name, indices, num_indices, value);
            }
            free(indices);
        }
        else
        {
            VarType vtype = runtime_get_variable_type(ctx->runtime, var->var_name);
            if (vtype == VAR_STRING)
            {
                if (dtype == VAR_STRING && str_val)
                    runtime_set_string_variable(ctx->runtime, var->var_name, str_val);
                else
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.15g", num_val);
                    runtime_set_string_variable(ctx->runtime, var->var_name, buf);
                }
            }
            else
            {
                double value = (dtype == VAR_STRING && str_val) ? strtod(str_val, NULL) : num_val;
                runtime_set_variable(ctx->runtime, var->var_name, value);
            }
        }

        if (str_val)
            free(str_val);
    }

    return 0;
}

/* Execute DATA statement */
static int execute_data_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)ctx;
    (void)stmt;
    return 0;
}

static int execute_restore_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt && stmt->target_line > 0)
    {
        runtime_data_reset_to_line(ctx->runtime, stmt->target_line);
    }
    else
    {
        runtime_data_reset(ctx->runtime);
    }
    return 0;
}

static int execute_def_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    VarType type = VAR_DOUBLE;
    if (stmt->type == STMT_DEFINT)
        type = VAR_INTEGER;
    else if (stmt->type == STMT_DEFSNG)
        type = VAR_SINGLE;
    else if (stmt->type == STMT_DEFDBL)
        type = VAR_DOUBLE;
    else if (stmt->type == STMT_DEFSTR)
        type = VAR_STRING;

    for (int i = 0; i < stmt->num_exprs; i++)
    {
        ASTExpr *expr = stmt->exprs[i];
        if (!expr || expr->type != EXPR_STRING || !expr->str_value)
            continue;

        char start = expr->str_value[0];
        char end = start;
        if (strlen(expr->str_value) == 3 && expr->str_value[1] == '-')
        {
            end = expr->str_value[2];
        }
        start = (char)toupper((unsigned char)start);
        end = (char)toupper((unsigned char)end);

        if (start < 'A' || start > 'Z' || end < 'A' || end > 'Z' || start > end)
        {
            return -BASIC_ERR_SYNTAX_ERROR;
        }

        runtime_set_def_range(ctx->runtime, type, start, end);
    }

    return 0;
}

static int execute_def_fn_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || ctx == NULL || ctx->runtime == NULL)
    {
        return 0;
    }

    /* Expected structure:
       exprs[0] = function name (string)
       exprs[1..n-1] = parameter names (strings)
       exprs[n] = body expression (the actual expression to evaluate)
    */

    if (stmt->num_exprs < 2)
    {
        return -BASIC_ERR_SYNTAX_ERROR;
    }

    /* Get function name */
    ASTExpr *name_expr = stmt->exprs[0];
    if (!name_expr || name_expr->type != EXPR_STRING || !name_expr->str_value)
    {
        return -BASIC_ERR_SYNTAX_ERROR;
    }

    const char *fn_name = name_expr->str_value;

    /* Count parameters (all string expressions before the last one) */
    int num_params = stmt->num_exprs - 2;

    /* Last expression is the body */
    ASTExpr *body_expr = stmt->exprs[stmt->num_exprs - 1];

    /* Collect parameter names */
    const char **params = NULL;
    if (num_params > 0)
    {
        params = (const char **)xmalloc(num_params * sizeof(const char *));
        for (int i = 0; i < num_params; i++)
        {
            ASTExpr *param_expr = stmt->exprs[i + 1];
            if (!param_expr || param_expr->type != EXPR_STRING || !param_expr->str_value)
            {
                free(params);
                return -BASIC_ERR_SYNTAX_ERROR;
            }
            params[i] = param_expr->str_value;
        }
    }

    /* Register the function */
    int success = runtime_define_function(ctx->runtime, fn_name, params, num_params, body_expr);

    if (params)
    {
        free(params);
    }

    return success ? 0 : -BASIC_ERR_SYNTAX_ERROR;
}

static int execute_on_error_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    runtime_set_error_handler(ctx->runtime, stmt->target_line);
    return 0;
}

static int execute_resume_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    if (!runtime_is_in_error_handler(ctx->runtime))
    {
        fprintf(stderr, "?RESUME WITHOUT ERROR\n");
        return 0;
    }

    runtime_set_in_error_handler(ctx->runtime, 0);

    int error_line = runtime_get_error_line(ctx->runtime);

    /* Clear error so resumed code does not see old ERR/ERL and abort */
    runtime_clear_error(ctx->runtime);

    if (stmt->mode == 1)
    {
        int error_index = find_program_line(ctx->program, error_line);
        if (error_index >= 0)
        {
            ctx->next_line_index = error_index + 1;
        }
        return 0;
    }

    if (stmt->mode == 2 && stmt->target_line > 0)
    {
        int target_index = find_program_line(ctx->program, stmt->target_line);
        if (target_index < 0)
        {
            return -BASIC_ERR_UNDEFINED_LINE;
        }
        ctx->next_line_index = target_index;
        return 0;
    }

    if (error_line > 0)
    {
        int error_index = find_program_line(ctx->program, error_line);
        if (error_index >= 0)
        {
            ctx->next_line_index = error_index;
        }
    }

    return 0;
}

static int execute_error_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return -BASIC_ERR_SYNTAX_ERROR;
    }

    int error_code = (int)eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    if (error_code <= 0)
    {
        error_code = BASIC_ERR_SYNTAX_ERROR;
    }

    int line_number = ctx->program->lines[ctx->current_line_index]->line_number;
    runtime_set_error(ctx->runtime, error_code, line_number);
    return -error_code;
}

static int execute_sleep_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    double seconds = eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    if (seconds < 0)
    {
        seconds = 0;
    }

    /* Convert to microseconds for usleep */
    unsigned int usec = (unsigned int)(seconds * 1000000);

    if (usec > 0)
    {
        usleep(usec);
    }

    return 0;
}

/* BEEP duration_ms [, freq]: freq 0=low, 1=mid (default), 2=high; or "LOW","MID","HIGH" */
static int execute_beep_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0)
    {
        return 0;
    }

    double duration_ms = eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    if (duration_ms < 0)
    {
        duration_ms = 0;
    }

    int freq_hz = 440; /* default mid */
    if (stmt->num_exprs >= 2 && stmt->exprs[1] != NULL)
    {
        if (is_string_expr(stmt->exprs[1]))
        {
            char *s = eval_string_expr(ctx->runtime, stmt->exprs[1]);
            if (s)
            {
                if (strcasecmp(s, "LOW") == 0)
                    freq_hz = 200;
                else if (strcasecmp(s, "MID") == 0)
                    freq_hz = 440;
                else if (strcasecmp(s, "HIGH") == 0)
                    freq_hz = 880;
                free(s);
            }
        }
        else
        {
            double val = eval_numeric_expr(ctx->runtime, stmt->exprs[1]);
            if (val >= 20 && val <= 4000)
                freq_hz = (int)val;
            else if (val >= 0 && val <= 2)
                freq_hz = (val < 0.5) ? 200 : (val < 1.5) ? 440
                                                          : 880;
            else if (val < 20)
                freq_hz = 20;
            else
                freq_hz = 4000;
        }
    }

    /* Use termio_beep for audible tone (SDL audio or fallback) */
    termio_beep((int)duration_ms, freq_hz);
    return 0;
}

static int execute_sound_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        /* SOUND requires at least frequency and duration */
        return 0;
    }

    double base_freq = eval_numeric_expr(ctx->runtime, stmt->exprs[0]);

    /* Clamp base frequency to valid range */
    if (base_freq < 20)
        base_freq = 20;
    if (base_freq > 4000)
        base_freq = 4000;

    /* Check if traditional format (2 exprs) or harmonic format (3+ exprs) */
    if (stmt->num_exprs == 2)
    {
        /* Traditional: SOUND frequency, duration */
        double duration_ms = eval_numeric_expr(ctx->runtime, stmt->exprs[1]);

        if (duration_ms < 0)
            duration_ms = 0;
        if (duration_ms > 5000)
            duration_ms = 5000;

        termio_beep((int)duration_ms, (int)base_freq);
    }
    else
    {
        /* Harmonic format: SOUND base_freq; h1, i1; h2, i2; ...; duration
           Last expression is duration, pairs before are (harmonic_num, intensity) */

        double duration_ms = eval_numeric_expr(ctx->runtime, stmt->exprs[stmt->num_exprs - 1]);
        if (duration_ms < 0)
            duration_ms = 0;
        if (duration_ms > 5000)
            duration_ms = 5000;

        /* Number of harmonic pairs: (total_exprs - 1 base - 1 duration) / 2 */
        int num_harmonics = (stmt->num_exprs - 2) / 2;
        if (num_harmonics <= 0)
        {
            /* Fallback to basic beep if no harmonics specified */
            termio_beep((int)duration_ms, (int)base_freq);
            return 0;
        }

        /* Allocate arrays for harmonics and intensities */
        int *harmonics = (int *)malloc(num_harmonics * sizeof(int));
        double *intensities = (double *)malloc(num_harmonics * sizeof(double));

        if (!harmonics || !intensities)
        {
            free(harmonics);
            free(intensities);
            return 0;
        }

        /* Extract harmonic pairs from expressions */
        for (int i = 0; i < num_harmonics; i++)
        {
            int harmonic_idx = 1 + (i * 2);
            int intensity_idx = 1 + (i * 2) + 1;

            if (harmonic_idx < stmt->num_exprs && intensity_idx < stmt->num_exprs)
            {
                harmonics[i] = (int)eval_numeric_expr(ctx->runtime, stmt->exprs[harmonic_idx]);
                intensities[i] = eval_numeric_expr(ctx->runtime, stmt->exprs[intensity_idx]);

                /* Clamp harmonic number to positive */
                if (harmonics[i] <= 0)
                    harmonics[i] = 1;

                /* Clamp intensity to 0-1 range */
                if (intensities[i] < 0.0)
                    intensities[i] = 0.0;
                if (intensities[i] > 1.0)
                    intensities[i] = 1.0;
            }
            else
            {
                /* Safety: use fundamental if missing */
                harmonics[i] = 1;
                intensities[i] = 1.0;
            }
        }

        /* Call harmonics synthesis function */
        termio_sound_harmonics((int)base_freq, harmonics, intensities, num_harmonics, (int)duration_ms);

        free(harmonics);
        free(intensities);
    }

    return 0;
}

static int execute_cls_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)ctx;  /* Unused */
    (void)stmt; /* Unused */
    termio_clear();
    return 0;
}

static int execute_clear_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)stmt; /* Memory size argument ignored for now */

    /* CLEAR clears all variables and arrays */
    if (ctx && ctx->runtime)
    {
        runtime_clear_all(ctx->runtime);
    }

    return 0;
}

static int execute_delete_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 1)
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_SYNTAX_ERROR, stmt ? stmt->line_number : 0);
        return 0;
    }

    int start_line, end_line;

    /* Check for special markers */
    double start_val = stmt->exprs[0]->num_value;

    if (start_val == -1.0)
    {
        /* DELETE . (current line) - use runtime's last_entered_line */
        int last_line = runtime_get_last_entered_line(ctx->runtime);
        if (last_line <= 0)
        {
            termio_write("?ILLEGAL LINE NUMBER\n");
            return 0;
        }
        start_line = last_line;
        end_line = last_line;
    }
    else if (start_val == -2.0)
    {
        /* DELETE -m (delete from first to m) */
        if (stmt->num_exprs < 2)
        {
            runtime_set_error(ctx->runtime, BASIC_ERR_SYNTAX_ERROR, stmt->line_number);
            return 0;
        }
        start_line = 1; /* Start from beginning */
        end_line = (int)stmt->exprs[1]->num_value;
    }
    else
    {
        /* Delete n or n-m */
        start_line = (int)start_val;
        if (stmt->num_exprs >= 2)
        {
            end_line = (int)stmt->exprs[1]->num_value;
        }
        else
        {
            end_line = start_line;
        }
    }

    /* Call the delete callback */
    DeleteCallback delete_fn = runtime_get_delete_callback(ctx->runtime);
    if (!delete_fn)
    {
        termio_write("?DELETE NOT AVAILABLE\n");
        return 0;
    }

    int result = delete_fn(start_line, end_line);
    if (result != 0)
    {
        termio_write("?ILLEGAL LINE NUMBER\n");
    }

    return 0;
}

static int execute_merge_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 1 || !ctx || !ctx->program)
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_SYNTAX_ERROR, stmt ? stmt->line_number : 0);
        return -1;
    }

    ASTExpr *filename_expr = stmt->exprs[0];
    if (filename_expr->type != EXPR_STRING || !filename_expr->str_value)
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_TYPE_MISMATCH, stmt->line_number);
        return -1;
    }

    /* Read and parse the merge file */
    FILE *fp = fopen(filename_expr->str_value, "r");
    if (!fp)
    {
        termio_printf("?FILE NOT FOUND\n");
        runtime_set_error(ctx->runtime, BASIC_ERR_DISK_BASIC, stmt->line_number);
        return -1;
    }

    /* Build program text from file */
    char *file_content = NULL;
    size_t file_capacity = 0;
    size_t file_size = 0;
    char line_buf[1024];

    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        size_t line_len = strlen(line_buf);
        if (file_size + line_len + 1 > file_capacity)
        {
            file_capacity = (file_capacity > 0) ? file_capacity * 2 : 4096;
            file_content = xrealloc(file_content, file_capacity);
        }
        strcpy(file_content + file_size, line_buf);
        file_size += line_len;
    }
    fclose(fp);

    if (!file_content || file_size == 0)
    {
        termio_printf("?EMPTY MERGE FILE\n");
        if (file_content)
            free(file_content);
        return 0;
    }

    /* Parse the merged program */
    Lexer *lexer = lexer_create(file_content);
    Token *tokens = lexer_tokenize(lexer);
    Parser *parser = parser_create(tokens, lexer_token_count(lexer));
    Program *merged_program = parse_program(parser);

    if (parser_has_error(parser))
    {
        termio_printf("?PARSE ERROR IN MERGE FILE\n");
        parser_free(parser);
        lexer_free(lexer);
        if (merged_program)
            ast_program_free(merged_program);
        free(file_content);
        return -1;
    }

    /* Merge parsed lines into current program */
    if (merged_program && merged_program->num_lines > 0)
    {
        for (int i = 0; i < merged_program->num_lines; i++)
        {
            ProgramLine *merged_line = merged_program->lines[i];
            int merged_line_num = merged_line->line_number;

            /* Check if line already exists */
            int existing_idx = -1;
            for (int j = 0; j < ctx->program->num_lines; j++)
            {
                if (ctx->program->lines[j]->line_number == merged_line_num)
                {
                    existing_idx = j;
                    termio_printf("MERGE: line %d overwritten\n", merged_line_num);
                    break;
                }
            }

            if (existing_idx >= 0)
            {
                /* Replace existing line */
                ast_stmt_free(ctx->program->lines[existing_idx]->stmt);
                free(ctx->program->lines[existing_idx]);
                ctx->program->lines[existing_idx] = merged_line;
                merged_program->lines[i] = NULL; /* Don't free it */
            }
            else
            {
                /* Insert new line in sorted order */
                if (ctx->program->num_lines >= ctx->program->capacity)
                {
                    ctx->program->capacity = (ctx->program->capacity > 0) ? ctx->program->capacity * 2 : 1024;
                    ctx->program->lines = xrealloc(ctx->program->lines,
                                                   (size_t)ctx->program->capacity * sizeof(ProgramLine *));
                }

                /* Find insertion point */
                int insert_idx = ctx->program->num_lines;
                for (int j = 0; j < ctx->program->num_lines; j++)
                {
                    if (ctx->program->lines[j]->line_number > merged_line_num)
                    {
                        insert_idx = j;
                        break;
                    }
                }

                /* Shift lines to make room */
                if (insert_idx < ctx->program->num_lines)
                {
                    memmove(&ctx->program->lines[insert_idx + 1], &ctx->program->lines[insert_idx],
                            (size_t)(ctx->program->num_lines - insert_idx) * sizeof(ProgramLine *));
                }

                ctx->program->lines[insert_idx] = merged_line;
                ctx->program->num_lines++;
                merged_program->lines[i] = NULL; /* Don't free it */
            }
        }
    }

    /* Clean up temp structures without freeing lines we adopted */
    for (int i = 0; i < merged_program->num_lines; i++)
    {
        merged_program->lines[i] = NULL; /* We adopted these */
    }
    ast_program_free(merged_program);

    parser_free(parser);
    lexer_free(lexer);
    free(file_content);

    /* Clear all variables and arrays (as per MERGE semantics) */
    runtime_clear_all(ctx->runtime);

    /* Close all open files */
    for (int i = 0; i < 10; i++)
    {
        runtime_close_file(ctx->runtime, i + 1);
    }

    return 0;
}

static int execute_open_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs == 0 || stmt->file_handle <= 0)
    {
        return -1;
    }

    ASTExpr *fname_expr = stmt->exprs[0];
    char *fname = (fname_expr && fname_expr->type == EXPR_STRING) ? fname_expr->str_value : NULL;
    if (!fname)
    {
        return -1;
    }

    const char *mode = "rb";
    if (stmt->mode == 2)
        mode = "wb";
    else if (stmt->mode == 3)
        mode = "ab";

    return runtime_open_file(ctx->runtime, stmt->file_handle, fname, mode) ? 0 : -1;
}

static int execute_close_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }
    if (stmt->file_handle > 0)
    {
        runtime_close_file(ctx->runtime, stmt->file_handle);
    }
    return 0;
}

static int execute_write_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->file_handle <= 0)
    {
        return -1;
    }

    FILE *fp = runtime_get_file(ctx->runtime, stmt->file_handle);
    if (!fp)
    {
        return -1;
    }

    for (int i = 0; i < stmt->num_exprs; i++)
    {
        ASTExpr *expr = stmt->exprs[i];
        if (expr_is_string(ctx->runtime, expr))
        {
            char *str_val = eval_string_expr(ctx->runtime, expr);
            fprintf(fp, "\"%s\"", str_val);
            free(str_val);
        }
        else
        {
            double num_val = eval_numeric_expr(ctx->runtime, expr);
            char buf[64];
            if (fabs(num_val) < 1e-10 && num_val != 0.0)
                snprintf(buf, sizeof(buf), "%.9e", num_val);
            else
                snprintf(buf, sizeof(buf), "%.15g", num_val);
            fprintf(fp, "%s", buf);
        }

        if (i < stmt->num_exprs - 1)
        {
            fprintf(fp, ",");
        }
    }

    fprintf(fp, "\n");
    fflush(fp);
    return 0;
}

static int execute_get_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->file_handle <= 0 || stmt->num_exprs == 0)
    {
        return -1;
    }

    int byte_val = 0;
    if (!runtime_file_get(ctx->runtime, stmt->file_handle, &byte_val))
    {
        return -1;
    }

    ASTExpr *var = stmt->exprs[0];
    if (var && var->var_name)
    {
        runtime_set_variable(ctx->runtime, var->var_name, (double)byte_val);
    }
    return 0;
}

static int execute_put_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->file_handle <= 0 || stmt->num_exprs == 0)
    {
        return -1;
    }

    int byte_val = (int)eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    return runtime_file_put(ctx->runtime, stmt->file_handle, byte_val) ? 0 : -1;
}

static int execute_poke_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 2)
    {
        return 0;
    }

    int addr = (int)eval_numeric_expr(ctx->runtime, stmt->exprs[0]);
    int value = 0;
    ASTExpr *value_expr = stmt->exprs[1];
    int treat_as_string = 0;
    if (value_expr)
    {
        if (value_expr->type == EXPR_STRING)
        {
            treat_as_string = 1;
        }
        else if (value_expr->type == EXPR_BINARY_OP && value_expr->op == OP_CONCAT)
        {
            treat_as_string = 1;
        }
        else if (value_expr->var_name && value_expr->var_name[0] != '\0')
        {
            size_t len = strlen(value_expr->var_name);
            if (len > 0 && value_expr->var_name[len - 1] == '$')
            {
                treat_as_string = 1;
            }
            else if (value_expr->type == EXPR_VAR || value_expr->type == EXPR_ARRAY)
            {
                if (runtime_get_variable_type(ctx->runtime, value_expr->var_name) == VAR_STRING)
                {
                    treat_as_string = 1;
                }
            }
        }
    }

    if (treat_as_string)
    {
        char *str_value = eval_string_expr(ctx->runtime, value_expr);
        value = (str_value && str_value[0] != '\0') ? (unsigned char)str_value[0] : 0;
        free(str_value);
    }
    else
    {
        value = (int)eval_numeric_expr(ctx->runtime, value_expr);
    }

    /* Store value to memory */
    runtime_poke(ctx->runtime, addr, value);

    return 0;
}

static int execute_save_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    if (stmt == NULL || stmt->num_exprs < 1)
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_SYNTAX_ERROR, stmt ? stmt->line_number : 0);
        return 0;
    }

    char *filename = eval_string_expr(ctx->runtime, stmt->exprs[0]);
    if (!filename || !filename[0])
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_SYNTAX_ERROR, stmt->line_number);
        if (filename)
            free(filename);
        return 0;
    }

    SaveCallback save_fn = runtime_get_save_callback(ctx->runtime);
    if (!save_fn)
    {
        termio_write("?SAVE NOT AVAILABLE\n");
        free(filename);
        return 0;
    }

    int result = save_fn(filename);
    if (result != 0)
    {
        runtime_set_error(ctx->runtime, BASIC_ERR_DISK_BASIC, stmt->line_number);
    }

    free(filename);
    return 0;
}

/* Execute END statement */
static int execute_end_stmt(ExecutionContext *ctx, ASTStmt *stmt)
{
    (void)ctx;
    (void)stmt;
    /* Terminate program execution */
    return 1; /* Non-zero return signals end */
}

/* Main program execution */
int execute_program(RuntimeState *state, Program *prog)
{
    if (state == NULL || prog == NULL)
    {
        return 0;
    }

    if (prog->num_lines == 0)
    {
        return 0;
    }

    ExecutionContext ctx;
    ctx.runtime = state;
    ctx.program = prog;
    ctx.current_line_index = 0;
    ctx.next_line_index = 1;
    ctx.next_stmt_override = NULL;
    ctx.skip_chained = 0;
    ctx.return_line_index = -1;
    ctx.error_code = 0;
    ctx.error_msg = NULL;
    ctx.for_stack = NULL;
    ctx.for_sp = 0;
    ctx.for_cap = 0;
    ctx.while_stack = NULL;
    ctx.while_sp = 0;
    ctx.while_cap = 0;
    ctx.proc_return_flag = 0;
    ctx.proc_return_value = 0.0;
    ctx.in_procedure = 0;

    preload_data(state, prog);

    /* Execute program line by line */
    int line_counter = 0;
    while (ctx.current_line_index < prog->num_lines)
    {
        if (g_interrupt_flag && *g_interrupt_flag)
        {
            *g_interrupt_flag = 0;
            break;
        }

        /* Process SDL events periodically to keep UI responsive */
        if (++line_counter % 10 == 0)
        {
            executor_process_events();
        }

        ASTStmt *stmt = ctx.next_stmt_override ? ctx.next_stmt_override : prog->lines[ctx.current_line_index]->stmt;
        ctx.next_stmt_override = NULL;

        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] Line %d\n", prog->lines[ctx.current_line_index]->line_number);
        }

        /* TRON: Print line number if trace is on */
        if (runtime_get_trace(state))
        {
            termio_printf("[%d]\n", prog->lines[ctx.current_line_index]->line_number);
            termio_present();
        }

        if (stmt != NULL)
        {
            int result = execute_stmt_internal(&ctx, stmt);

            if (result != 0)
            {
                if (result > 0)
                {
                    /* END statement */
                    break;
                }

                int error_code = -result;
                int error_line = prog->lines[ctx.current_line_index]->line_number;
                runtime_set_error(state, error_code, error_line);

                int handler_line = runtime_get_error_handler(state);
                if (handler_line > 0 && !runtime_is_in_error_handler(state))
                {
                    runtime_set_in_error_handler(state, 1);
                    int handler_index = find_program_line(prog, handler_line);
                    if (handler_index >= 0)
                    {
                        ctx.next_line_index = handler_index;
                    }
                    else
                    {
                        error_print(error_code, error_line);
                        break;
                    }
                }
                else
                {
                    error_print(error_code, error_line);
                    break;
                }
            }
        }

        /* Move to next line (unless GOTO/GOSUB changed it) */
        if (ctx.next_line_index == ctx.current_line_index + 1)
        {
            ctx.current_line_index++;
        }
        else
        {
            ctx.current_line_index = ctx.next_line_index;
        }

        ctx.next_line_index = ctx.current_line_index + 1;
    }

    if (ctx.for_stack)
    {
        for (int i = 0; i < ctx.for_sp; i++)
        {
            free(ctx.for_stack[i].var_name);
        }
        free(ctx.for_stack);
    }

    if (ctx.while_stack)
    {
        free(ctx.while_stack);
    }

    return 0;
}

/* Execute program starting from a specific line number */
int execute_program_from_line(RuntimeState *state, Program *prog, int start_line_num)
{
    if (state == NULL || prog == NULL)
    {
        return 0;
    }

    if (prog->num_lines == 0)
    {
        return 0;
    }

    /* Find the index of the starting line */
    int start_index = find_program_line(prog, start_line_num);
    if (start_index < 0)
    {
        /* Line number not found */
        error_print(7, start_line_num); /* Error 7: UNDEFINED LINE */
        return 1;
    }

    ExecutionContext ctx;
    ctx.runtime = state;
    ctx.program = prog;
    ctx.current_line_index = start_index;
    ctx.next_line_index = start_index + 1;
    ctx.next_stmt_override = NULL;
    ctx.skip_chained = 0;
    ctx.return_line_index = -1;
    ctx.error_code = 0;
    ctx.error_msg = NULL;
    ctx.for_stack = NULL;
    ctx.for_sp = 0;
    ctx.for_cap = 0;
    ctx.while_stack = NULL;
    ctx.while_sp = 0;
    ctx.while_cap = 0;
    ctx.proc_return_flag = 0;
    ctx.proc_return_value = 0.0;
    ctx.in_procedure = 0;

    preload_data(state, prog);

    /* Execute program line by line */
    int line_counter = 0;
    while (ctx.current_line_index < prog->num_lines)
    {
        if (g_interrupt_flag && *g_interrupt_flag)
        {
            *g_interrupt_flag = 0;
            break;
        }

        /* Process SDL events periodically to keep UI responsive */
        if (++line_counter % 10 == 0)
        {
            executor_process_events();
        }

        ASTStmt *stmt = ctx.next_stmt_override ? ctx.next_stmt_override : prog->lines[ctx.current_line_index]->stmt;
        ctx.next_stmt_override = NULL;

        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[AST] Line %d\n", prog->lines[ctx.current_line_index]->line_number);
        }

        /* TRON: Print line number if trace is on */
        if (runtime_get_trace(state))
        {
            termio_printf("[%d]\n", prog->lines[ctx.current_line_index]->line_number);
            termio_present();
        }

        if (stmt != NULL)
        {
            int result = execute_stmt_internal(&ctx, stmt);

            if (result != 0)
            {
                if (result > 0)
                {
                    /* END statement */
                    break;
                }

                int error_code = -result;
                int error_line = prog->lines[ctx.current_line_index]->line_number;
                runtime_set_error(state, error_code, error_line);

                int handler_line = runtime_get_error_handler(state);
                if (handler_line > 0 && !runtime_is_in_error_handler(state))
                {
                    runtime_set_in_error_handler(state, 1);
                    int handler_index = find_program_line(prog, handler_line);
                    if (handler_index >= 0)
                    {
                        ctx.next_line_index = handler_index;
                    }
                    else
                    {
                        error_print(error_code, error_line);
                        break;
                    }
                }
                else
                {
                    error_print(error_code, error_line);
                    break;
                }
            }
        }

        /* Move to next line (unless GOTO/GOSUB changed it) */
        if (ctx.next_line_index == ctx.current_line_index + 1)
        {
            ctx.current_line_index++;
        }
        else
        {
            ctx.current_line_index = ctx.next_line_index;
        }

        ctx.next_line_index = ctx.current_line_index + 1;
    }

    if (ctx.for_stack)
    {
        for (int i = 0; i < ctx.for_sp; i++)
        {
            free(ctx.for_stack[i].var_name);
        }
        free(ctx.for_stack);
    }

    if (ctx.while_stack)
    {
        free(ctx.while_stack);
    }

    return 0;
}

/* Execute a single statement (public interface) */
int execute_statement(RuntimeState *state, ASTStmt *stmt, Program *prog)
{
    if (state == NULL || stmt == NULL || prog == NULL)
    {
        return 0;
    }

    ExecutionContext ctx;
    ctx.runtime = state;
    ctx.program = prog;
    ctx.current_line_index = 0;
    ctx.next_line_index = 1;
    ctx.next_stmt_override = NULL;
    ctx.skip_chained = 0;
    ctx.return_line_index = -1;
    ctx.error_code = 0;
    ctx.error_msg = NULL;
    ctx.for_stack = NULL;
    ctx.for_sp = 0;
    ctx.for_cap = 0;
    ctx.while_stack = NULL;
    ctx.while_sp = 0;
    ctx.while_cap = 0;
    ctx.proc_return_flag = 0;
    ctx.proc_return_value = 0.0;
    ctx.in_procedure = 0;

    return execute_stmt_internal(&ctx, stmt);
}

/* Accessor for trace support to avoid exposing ExecutionContext struct layout */
RuntimeState *executor_get_runtime(ExecutionContext *ctx)
{
    if (!ctx)
    {
        return NULL;
    }
    return ctx->runtime;
}
