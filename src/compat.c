#include "compat.h"
#include "common.h"
#include "ast.h"
#include "termio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global compatibility state */
CompatState *g_compat_state = NULL;

/* TRS-80 Level II BASIC keywords (authentic set) */
static const char *trs80_keywords[] = {
    "PRINT", "INPUT", "LET", "IF", "THEN", "ELSE",
    "GOTO", "GOSUB", "RETURN", "FOR", "TO", "STEP", "NEXT",
    "DIM", "READ", "DATA", "RESTORE",
    "REM", "END", "STOP",
    "ON", "POKE", "OUT",
    "CLS", "SET", "RESET", "POINT",
    "LINE", "SOUND",
    "TRON", "TROFF",
    "AND", "OR", "NOT",
    "OPEN", "CLOSE", "GET", "PUT",
    NULL};

/* TRS-80 Level II BASIC functions */
static const char *trs80_functions[] = {
    "ABS", "ASC", "ATN", "CHR$", "COS", "EXP",
    "INKEY$", "INP", "INT", "LEFT$", "LEN", "LOG",
    "MID$", "PEEK", "RIGHT$", "RND", "SGN", "SIN",
    "SQR", "STR$", "STRING$", "TAN", "VAL",
    "LOC", "LOF", "EOF",
    "USR", "VARPTR",
    NULL};

CompatState *compat_init(int strict_mode)
{
    CompatState *state = xmalloc(sizeof(CompatState));
    state->strict_mode = strict_mode;
    state->violations = NULL;
    state->violation_count = 0;
    return state;
}

void compat_free(CompatState *state)
{
    if (!state)
        return;

    compat_clear_violations(state);
    free(state);
}

void compat_record_violation(CompatState *state, CompatViolationType type,
                             int line_number, const char *description)
{
    if (!state)
        return;

    CompatViolation *violation = xmalloc(sizeof(CompatViolation));
    violation->type = type;
    violation->line_number = line_number;
    violation->description = xstrdup(description);
    violation->next = state->violations;
    state->violations = violation;
    state->violation_count++;
}

int compat_is_strict(CompatState *state)
{
    return state ? state->strict_mode : 0;
}

void compat_print_violations(CompatState *state)
{
    if (!state || !state->violations)
    {
        termio_write("No compatibility issues found.\n");
        termio_write("Program appears compatible with TRS-80 Level II BASIC.\n");
        return;
    }

    termio_write("\n=== TRS-80 COMPATIBILITY REPORT ===\n\n");
    termio_printf("Found %d compatibility issue%s:\n\n",
                  state->violation_count,
                  state->violation_count == 1 ? "" : "s");

    /* Print violations in reverse order (they were added in reverse) */
    int count = state->violation_count;
    CompatViolation **violations_array = xmalloc(sizeof(CompatViolation *) * count);

    CompatViolation *v = state->violations;
    for (int i = count - 1; i >= 0 && v; i--, v = v->next)
    {
        violations_array[i] = v;
    }

    for (int i = 0; i < count; i++)
    {
        v = violations_array[i];
        const char *type_str = "";

        switch (v->type)
        {
        case COMPAT_ARRAY_WITHOUT_DIM:
            type_str = "ARRAY WITHOUT DIM";
            break;
        case COMPAT_MODERN_KEYWORD:
            type_str = "MODERN KEYWORD";
            break;
        case COMPAT_ERROR_HANDLING:
            type_str = "ERROR HANDLING";
            break;
        case COMPAT_LONG_LINE:
            type_str = "LINE TOO LONG";
            break;
        case COMPAT_EXTENDED_FUNCTION:
            type_str = "EXTENDED FUNCTION";
            break;
        case COMPAT_FILE_MODE:
            type_str = "ADVANCED FILE I/O";
            break;
        case COMPAT_LINE_NUMBER_RANGE:
            type_str = "LINE NUMBER OUT OF RANGE";
            break;
        }

        if (v->line_number > 0)
        {
            termio_printf("%3d. Line %5d: [%s] %s\n",
                          i + 1, v->line_number, type_str, v->description);
        }
        else
        {
            termio_printf("%3d. [%s] %s\n", i + 1, type_str, v->description);
        }
    }

    free(violations_array);

    termio_write("\n=== RECOMMENDATIONS ===\n");
    termio_write("- Use DIM to declare all arrays\n");
    termio_write("- Avoid modern keywords (SLEEP, DEFINT, ERROR, RESUME)\n");
    termio_write("- Keep line numbers between 0-65529\n");
    termio_write("- Use only standard TRS-80 Level II functions\n");
    termio_write("\nRun with --strict flag to enforce TRS-80 compatibility.\n\n");
}

void compat_clear_violations(CompatState *state)
{
    if (!state)
        return;

    CompatViolation *v = state->violations;
    while (v)
    {
        CompatViolation *next = v->next;
        free(v->description);
        free(v);
        v = next;
    }
    state->violations = NULL;
    state->violation_count = 0;
}

int compat_is_trs80_keyword(const char *keyword)
{
    for (int i = 0; trs80_keywords[i] != NULL; i++)
    {
        if (strcasecmp(keyword, trs80_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int compat_is_trs80_function(const char *function)
{
    for (int i = 0; trs80_functions[i] != NULL; i++)
    {
        if (strcasecmp(function, trs80_functions[i]) == 0)
            return 1;
    }
    return 0;
}

/* Check for undeclared arrays in program */
static void check_array_usage(ASTExpr *expr, const char **declared_arrays,
                              int declared_count, CompatState *state, int line_num)
{
    if (!expr)
        return;

    /* Check if this is an array access expression */
    if (expr->type == EXPR_ARRAY && expr->var_name != NULL)
    {
        /* Check if this array is declared */
        int found = 0;
        for (int i = 0; i < declared_count; i++)
        {
            if (strcasecmp(declared_arrays[i], expr->var_name) == 0)
            {
                found = 1;
                break;
            }
        }

        if (!found)
        {
            /* Array used without DIM */
            char desc[256];
            snprintf(desc, sizeof(desc), "Array '%s' used without DIM statement", expr->var_name);
            compat_record_violation(state, COMPAT_ARRAY_WITHOUT_DIM, line_num, desc);
        }
    }

    /* Recursively check children */
    for (int i = 0; i < expr->num_children; i++)
    {
        check_array_usage(expr->children[i], declared_arrays, declared_count, state, line_num);
    }
}

static void collect_dims(ASTStmt *stmt, const char **declared_arrays, int *declared_count)
{
    if (!stmt)
        return;

    /* Collect DIM statements */
    if (stmt->type == STMT_DIM)
    {
        /* DIM has array expressions as children */
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            ASTExpr *expr = stmt->exprs[i];
            if (expr && expr->type == EXPR_ARRAY && expr->var_name != NULL)
            {
                /* Check if already in list */
                int found = 0;
                for (int j = 0; j < *declared_count; j++)
                {
                    if (strcasecmp(declared_arrays[j], expr->var_name) == 0)
                    {
                        found = 1;
                        break;
                    }
                }
                if (!found && *declared_count < 1000)
                {
                    declared_arrays[*declared_count] = expr->var_name;
                    (*declared_count)++;
                }
            }
        }
    }

    /* Recursively collect from nested statements */
    if (stmt->body)
        collect_dims(stmt->body, declared_arrays, declared_count);

    if (stmt->else_body)
        collect_dims(stmt->else_body, declared_arrays, declared_count);

    if (stmt->next)
        collect_dims(stmt->next, declared_arrays, declared_count);
}

static void check_statement(ASTStmt *stmt, const char **declared_arrays,
                            int declared_count, CompatState *state)
{
    if (!stmt)
        return;

    /* Check all expressions in this statement */
    for (int i = 0; i < stmt->num_exprs; i++)
    {
        check_array_usage(stmt->exprs[i], declared_arrays, declared_count, state, stmt->line_number);
    }

    /* Check body statements (IF, FOR, etc.) */
    if (stmt->body)
        check_statement(stmt->body, declared_arrays, declared_count, state);

    if (stmt->else_body)
        check_statement(stmt->else_body, declared_arrays, declared_count, state);

    /* Check next statement (colon-separated) */
    if (stmt->next)
        check_statement(stmt->next, declared_arrays, declared_count, state);
}

/* Scan program for arrays without DIM statements */
void compat_check_program_arrays(void *program_ptr, CompatState *state)
{
    Program *program = (Program *)program_ptr;

    if (!program || !state)
        return;

    /* Collect all declared arrays */
    const char *declared_arrays[1000];
    int declared_count = 0;

    /* First pass: collect all DIM statements */
    for (int i = 0; i < program->num_lines; i++)
    {
        ProgramLine *line = program->lines[i];
        if (line && line->stmt)
        {
            collect_dims(line->stmt, declared_arrays, &declared_count);
        }
    }

    /* Second pass: check for undeclared array usage */
    for (int i = 0; i < program->num_lines; i++)
    {
        ProgramLine *line = program->lines[i];
        if (line && line->stmt)
        {
            check_statement(line->stmt, declared_arrays, declared_count, state);
        }
    }
}
