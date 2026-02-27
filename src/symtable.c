#include "symtable.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Helper: Determine variable type from name suffix/defaults */
static VarType get_var_type_from_name(const char *name);
static void set_type_range(char start_letter, char end_letter, VarType type);

static VarType letter_types[26];
static int letter_types_initialized = 0;

/* Forward declarations for analysis */
static int analyze_program_line(SymbolTable *table, ProgramLine *line);
static int analyze_statement(SymbolTable *table, ASTStmt *stmt);
static int analyze_expression(SymbolTable *table, ASTExpr *expr);

SymbolTable *symtable_create(void)
{
    SymbolTable *table = xcalloc(1, sizeof(SymbolTable));
    table->capacity = 256;
    table->symbols = xmalloc(table->capacity * sizeof(Symbol));
    table->num_symbols = 0;

    if (!letter_types_initialized)
    {
        for (int i = 0; i < 26; i++)
        {
            letter_types[i] = VAR_DOUBLE;
        }
        letter_types_initialized = 1;
    }
    return table;
}

void symtable_free(SymbolTable *table)
{
    if (table == NULL)
    {
        return;
    }
    if (table->symbols != NULL)
    {
        for (int i = 0; i < table->num_symbols; i++)
        {
            if (table->symbols[i].name != NULL)
            {
                free(table->symbols[i].name);
            }
            if (table->symbols[i].dimensions != NULL)
            {
                free(table->symbols[i].dimensions);
            }
        }
        free(table->symbols);
    }
    free(table);
}

Symbol *symtable_lookup(SymbolTable *table, const char *name)
{
    if (table == NULL || name == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < table->num_symbols; i++)
    {
        if (strcmp(table->symbols[i].name, name) == 0)
        {
            return &table->symbols[i];
        }
    }

    return NULL;
}

void symtable_insert(SymbolTable *table, const char *name, VarType type)
{
    if (table == NULL || name == NULL)
    {
        return;
    }

    /* Check if already exists */
    Symbol *existing = symtable_lookup(table, name);
    if (existing != NULL)
    {
        /* Already defined, update type if needed */
        existing->type = type;
        return;
    }

    /* Expand capacity if needed */
    if (table->num_symbols >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = xrealloc(table->symbols,
                                  table->capacity * sizeof(Symbol));
    }

    /* Add new symbol */
    Symbol *sym = &table->symbols[table->num_symbols++];
    sym->name = xstrdup(name);
    sym->type = type;
    sym->is_array = 0;
    sym->dimensions = NULL;
    sym->num_dimensions = 0;
    sym->is_function = 0;
    sym->line_defined = 0;
}

void symtable_insert_array(SymbolTable *table, const char *name,
                           VarType type, int *dimensions, int num_dims)
{
    if (table == NULL || name == NULL)
    {
        return;
    }

    /* Check if already exists */
    Symbol *existing = symtable_lookup(table, name);
    if (existing != NULL)
    {
        /* Already defined as array, update if needed */
        if (existing->is_array && existing->dimensions != NULL)
        {
            free(existing->dimensions);
        }
        existing->type = type;
        existing->is_array = 1;
        existing->num_dimensions = num_dims;
        existing->dimensions = xmalloc(num_dims * sizeof(int));
        memcpy(existing->dimensions, dimensions, num_dims * sizeof(int));
        return;
    }

    /* Expand capacity if needed */
    if (table->num_symbols >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = xrealloc(table->symbols,
                                  table->capacity * sizeof(Symbol));
    }

    /* Add new array symbol */
    Symbol *sym = &table->symbols[table->num_symbols++];
    sym->name = xstrdup(name);
    sym->type = type;
    sym->is_array = 1;
    sym->dimensions = xmalloc(num_dims * sizeof(int));
    memcpy(sym->dimensions, dimensions, num_dims * sizeof(int));
    sym->num_dimensions = num_dims;
    sym->is_function = 0;
    sym->line_defined = 0;
}

static VarType get_var_type_from_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return VAR_DOUBLE; /* Default */
    }

    /* Check suffix */
    int len = strlen(name);
    char last = name[len - 1];

    if (last == '$')
    {
        return VAR_STRING;
    }
    else if (last == '%')
    {
        return VAR_INTEGER;
    }
    else if (last == '!')
    {
        return VAR_SINGLE;
    }
    else if (last == '#')
    {
        return VAR_DOUBLE;
    }

    char first = (char)toupper((unsigned char)name[0]);
    if (first >= 'A' && first <= 'Z')
    {
        return letter_types[first - 'A'];
    }

    return VAR_DOUBLE;
}

static void set_type_range(char start_letter, char end_letter, VarType type)
{
    if (start_letter > end_letter)
    {
        return;
    }

    for (char c = start_letter; c <= end_letter; c++)
    {
        if (c >= 'A' && c <= 'Z')
        {
            letter_types[c - 'A'] = type;
        }
    }
}
int symtable_analyze_program(SymbolTable *table, Program *prog)
{
    if (table == NULL || prog == NULL)
    {
        return -1;
    }

    /* Analyze each program line */
    for (int i = 0; i < prog->num_lines; i++)
    {
        if (analyze_program_line(table, prog->lines[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

static int analyze_program_line(SymbolTable *table, ProgramLine *line)
{
    if (line == NULL || line->stmt == NULL)
    {
        return 0;
    }

    return analyze_statement(table, line->stmt);
}

static int analyze_statement(SymbolTable *table, ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return 0;
    }

    switch (stmt->type)
    {
    case STMT_LET:
        /* Assignment: analyze LHS and RHS */
        if (stmt->num_exprs >= 2)
        {
            ASTExpr *lhs = stmt->exprs[0];
            ASTExpr *rhs = stmt->exprs[1];

            /* Register variable if not already defined */
            if (lhs && lhs->var_name)
            {
                VarType type = get_var_type_from_name(lhs->var_name);

                if (lhs->type == EXPR_ARRAY)
                {
                    /* Array assignment - should already be defined via DIM */
                    Symbol *sym = symtable_lookup(table, lhs->var_name);
                    if (sym == NULL)
                    {
                        /* Implicit array (not recommended, but some BASICs allow) */
                        symtable_insert(table, lhs->var_name, type);
                    }
                }
                else
                {
                    /* Simple variable */
                    symtable_insert(table, lhs->var_name, type);
                }
            }

            /* Analyze RHS expression */
            analyze_expression(table, rhs);
        }
        break;

    case STMT_INPUT:
        /* INPUT: register variables being input */
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            ASTExpr *expr = stmt->exprs[i];
            if (expr && expr->type == EXPR_VAR && expr->var_name)
            {
                VarType type = get_var_type_from_name(expr->var_name);
                symtable_insert(table, expr->var_name, type);
            }
        }
        break;

    case STMT_DIM:
        /* DIM: register array declarations */
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            ASTExpr *expr = stmt->exprs[i];
            if (expr && expr->type == EXPR_ARRAY && expr->var_name)
            {
                VarType type = get_var_type_from_name(expr->var_name);

                /* Extract dimensions */
                int num_dims = expr->num_children;
                int *dims = xmalloc(num_dims * sizeof(int));

                for (int j = 0; j < num_dims; j++)
                {
                    /* Dimensions are expressions, default to 10 for analysis */
                    dims[j] = 10;
                }

                symtable_insert_array(table, expr->var_name, type, dims, num_dims);
                free(dims);
            }
        }
        break;

    case STMT_FOR:
        /* FOR: register loop variable */
        if (stmt->num_exprs >= 1)
        {
            ASTExpr *var = stmt->exprs[0];
            if (var && var->var_name)
            {
                VarType type = get_var_type_from_name(var->var_name);
                symtable_insert(table, var->var_name, type);
            }

            /* Analyze start, end, step expressions */
            for (int i = 1; i < stmt->num_exprs; i++)
            {
                analyze_expression(table, stmt->exprs[i]);
            }
        }
        break;

    case STMT_PRINT:
        /* PRINT: analyze all expressions */
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            analyze_expression(table, stmt->exprs[i]);
        }
        break;

    case STMT_IF:
        /* IF: analyze condition and body */
        if (stmt->num_exprs >= 1)
        {
            analyze_expression(table, stmt->exprs[0]);
        }
        if (stmt->body)
        {
            analyze_statement(table, stmt->body);
        }
        break;

    case STMT_NEXT:
    case STMT_GOTO:
    case STMT_GOSUB:
    case STMT_RETURN:
    case STMT_END:
    case STMT_REM:
        /* These don't introduce new variables */
        break;

    case STMT_DEFINT:
    case STMT_DEFSNG:
    case STMT_DEFDBL:
    case STMT_DEFSTR:
    {
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
            if (start >= 'A' && start <= 'Z' && end >= 'A' && end <= 'Z' && start <= end)
            {
                set_type_range(start, end, type);
            }
        }
        break;
    }

    default:
        break;
    }

    return 0;
}

static int analyze_expression(SymbolTable *table, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0;
    }

    switch (expr->type)
    {
    case EXPR_VAR:
        /* Variable reference */
        if (expr->var_name)
        {
            Symbol *sym = symtable_lookup(table, expr->var_name);
            if (sym == NULL)
            {
                /* Variable used but not defined - auto-create */
                VarType type = get_var_type_from_name(expr->var_name);
                symtable_insert(table, expr->var_name, type);
            }
        }
        break;

    case EXPR_ARRAY:
        /* Array reference */
        if (expr->var_name)
        {
            Symbol *sym = symtable_lookup(table, expr->var_name);
            if (sym == NULL)
            {
                /* Array used but not defined */
                VarType type = get_var_type_from_name(expr->var_name);
                symtable_insert(table, expr->var_name, type);
            }

            /* Analyze subscript expressions */
            for (int i = 0; i < expr->num_children; i++)
            {
                analyze_expression(table, expr->children[i]);
            }
        }
        break;

    case EXPR_FUNC_CALL:
        /* Function call - analyze arguments */
        for (int i = 0; i < expr->num_children; i++)
        {
            analyze_expression(table, expr->children[i]);
        }
        break;

    case EXPR_BINARY_OP:
    case EXPR_UNARY_OP:
        /* Analyze operands */
        for (int i = 0; i < expr->num_children; i++)
        {
            analyze_expression(table, expr->children[i]);
        }
        break;

    case EXPR_NUMBER:
    case EXPR_STRING:
        /* Literals - no analysis needed */
        break;

    default:
        break;
    }

    return 0;
}
