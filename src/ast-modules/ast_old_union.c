#include "ast.h"

/*
 * Expression creation
 */

ASTExpr *ast_expr_create(ExprType type)
{
    ASTExpr *expr = xcalloc(1, sizeof(ASTExpr));
    expr->type = type;
    expr->line_number = 0;
    expr->column_number = 0;
    expr->inferred_type = VAR_UNDEFINED;
    return expr;
}

/*
 * Statement creation
 */

ASTStmt *ast_stmt_create(StmtType type)
{
    ASTStmt *stmt = xcalloc(1, sizeof(ASTStmt));
    stmt->type = type;
    stmt->line_number = 0;
    return stmt;
}

/*
 * Program creation
 */

Program *ast_program_create(void)
{
    Program *prog = xcalloc(1, sizeof(Program));
    prog->capacity = 1024;
    prog->lines = xmalloc(prog->capacity * sizeof(ProgramLine));
    prog->num_lines = 0;
    return prog;
}

/*
 * Expression destruction (recursive)
 */

void ast_expr_free(ASTExpr *expr)
{
    if (expr == NULL)
    {
        return;
    }

    switch (expr->type)
    {
    case EXPR_STRING:
        if (expr->data.string.value != NULL)
        {
            free(expr->data.string.value);
        }
        break;

    case EXPR_VARIABLE:
        if (expr->data.variable.name != NULL)
        {
            free(expr->data.variable.name);
        }
        break;

    case EXPR_ARRAY_ACCESS:
        if (expr->data.array_access.name != NULL)
        {
            free(expr->data.array_access.name);
        }
        if (expr->data.array_access.indices != NULL)
        {
            for (int i = 0; i < expr->data.array_access.num_indices; i++)
            {
                ast_expr_free(expr->data.array_access.indices[i]);
            }
            free(expr->data.array_access.indices);
        }
        break;

    case EXPR_BINARY_OP:
        ast_expr_free(expr->data.binary_op.left);
        ast_expr_free(expr->data.binary_op.right);
        break;

    case EXPR_UNARY_OP:
        ast_expr_free(expr->data.unary_op.operand);
        break;

    case EXPR_FUNCTION_CALL:
        if (expr->data.function_call.func_name != NULL)
        {
            free(expr->data.function_call.func_name);
        }
        if (expr->data.function_call.args != NULL)
        {
            for (int i = 0; i < expr->data.function_call.num_args; i++)
            {
                ast_expr_free(expr->data.function_call.args[i]);
            }
            free(expr->data.function_call.args);
        }
        break;

    case EXPR_CAST:
        ast_expr_free(expr->data.cast.expr);
        break;

    default:
        break;
    }

    free(expr);
}

/*
 * Statement destruction (recursive)
 */

void ast_stmt_free(ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return;
    }

    switch (stmt->type)
    {
    case STMT_PRINT:
        if (stmt->data.print_stmt.exprs != NULL)
        {
            for (int i = 0; i < stmt->data.print_stmt.num_exprs; i++)
            {
                ast_expr_free(stmt->data.print_stmt.exprs[i]);
            }
            free(stmt->data.print_stmt.exprs);
        }
        break;

    case STMT_INPUT:
        if (stmt->data.input_stmt.var_name != NULL)
        {
            free(stmt->data.input_stmt.var_name);
        }
        ast_expr_free(stmt->data.input_stmt.expr);
        break;

    case STMT_LET:
        if (stmt->data.let_stmt.var_name != NULL)
        {
            free(stmt->data.let_stmt.var_name);
        }
        ast_expr_free(stmt->data.let_stmt.expr);
        if (stmt->data.let_stmt.indices != NULL)
        {
            for (int i = 0; i < stmt->data.let_stmt.num_indices; i++)
            {
                ast_expr_free(stmt->data.let_stmt.indices[i]);
            }
            free(stmt->data.let_stmt.indices);
        }
        break;

    case STMT_IF:
        ast_expr_free(stmt->data.if_stmt.condition);
        ast_stmt_free(stmt->data.if_stmt.then_branch);
        ast_stmt_free(stmt->data.if_stmt.else_branch);
        break;

    case STMT_FOR:
        if (stmt->data.for_stmt.var_name != NULL)
        {
            free(stmt->data.for_stmt.var_name);
        }
        ast_expr_free(stmt->data.for_stmt.start);
        ast_expr_free(stmt->data.for_stmt.end);
        ast_expr_free(stmt->data.for_stmt.step);
        ast_stmt_free(stmt->data.for_stmt.body);
        break;

    case STMT_BLOCK:
        if (stmt->data.block_stmt.statements != NULL)
        {
            for (int i = 0; i < stmt->data.block_stmt.num_statements; i++)
            {
                ast_stmt_free(stmt->data.block_stmt.statements[i]);
            }
            free(stmt->data.block_stmt.statements);
        }
        break;

    case STMT_REM:
        if (stmt->data.rem_stmt.text != NULL)
        {
            free(stmt->data.rem_stmt.text);
        }
        break;

    default:
        break;
    }

    free(stmt);
}

/*
 * Program destruction
 */

void ast_program_free(Program *prog)
{
    if (prog == NULL)
    {
        return;
    }

    if (prog->lines != NULL)
    {
        for (int i = 0; i < prog->num_lines; i++)
        {
            for (int j = 0; j < prog->lines[i].num_statements; j++)
            {
                ast_stmt_free(prog->lines[i].statements[j]);
            }
            if (prog->lines[i].statements != NULL)
            {
                free(prog->lines[i].statements);
            }
        }
        free(prog->lines);
    }

    free(prog);
}

/*
 * Debug printing functions (stubs)
 */

void ast_expr_print(ASTExpr *expr)
{
    if (expr == NULL)
    {
        return;
    }
    printf("[EXPR: %s]", expr_type_name(expr->type));
}

void ast_stmt_print(ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return;
    }
    printf("[STMT: %s]", stmt_type_name(stmt->type));
}

void ast_program_print(Program *prog)
{
    if (prog == NULL)
    {
        return;
    }
    printf("[PROGRAM: %d lines]\n", prog->num_lines);
}

/*
 * Type name functions
 */

const char *stmt_type_name(StmtType type)
{
    switch (type)
    {
    case STMT_PRINT:
        return "PRINT";
    case STMT_INPUT:
        return "INPUT";
    case STMT_LET:
        return "LET";
    case STMT_IF:
        return "IF";
    case STMT_FOR:
        return "FOR";
    case STMT_NEXT:
        return "NEXT";
    case STMT_GOTO:
        return "GOTO";
    case STMT_GOSUB:
        return "GOSUB";
    case STMT_RETURN:
        return "RETURN";
    case STMT_READ:
        return "READ";
    case STMT_DATA:
        return "DATA";
    case STMT_DIM:
        return "DIM";
    case STMT_OPEN:
        return "OPEN";
    case STMT_CLOSE:
        return "CLOSE";
    case STMT_GET:
        return "GET";
    case STMT_PUT:
        return "PUT";
    case STMT_END:
        return "END";
    case STMT_REM:
        return "REM";
    case STMT_BLOCK:
        return "BLOCK";
    case STMT_DEFINT:
        return "DEFINT";
    case STMT_DEFSNG:
        return "DEFSNG";
    case STMT_DEFDBL:
        return "DEFDBL";
    case STMT_DEFSTR:
        return "DEFSTR";
    case STMT_ON_ERROR:
        return "ON_ERROR";
    case STMT_RESUME:
        return "RESUME";
    case STMT_RANDOMIZE:
        return "RANDOMIZE";
    case STMT_POKE:
        return "POKE";
    case STMT_CALL:
        return "CALL";
    case STMT_DEF_FN:
        return "DEF_FN";
    default:
        return "UNKNOWN";
    }
}

const char *expr_type_name(ExprType type)
{
    switch (type)
    {
    case EXPR_NUMBER:
        return "NUMBER";
    case EXPR_STRING:
        return "STRING";
    case EXPR_VARIABLE:
        return "VARIABLE";
    case EXPR_ARRAY_ACCESS:
        return "ARRAY_ACCESS";
    case EXPR_BINARY_OP:
        return "BINARY_OP";
    case EXPR_UNARY_OP:
        return "UNARY_OP";
    case EXPR_FUNCTION_CALL:
        return "FUNCTION_CALL";
    case EXPR_CAST:
        return "CAST";
    default:
        return "UNKNOWN";
    }
}

const char *op_type_name(OpType type)
{
    switch (type)
    {
    case OP_PLUS:
        return "+";
    case OP_MINUS:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    case OP_MOD:
        return "MOD";
    case OP_POWER:
        return "^";
    case OP_EQ:
        return "=";
    case OP_NE:
        return "<>";
    case OP_LT:
        return "<";
    case OP_LE:
        return "<=";
    case OP_GT:
        return ">";
    case OP_GE:
        return ">=";
    case OP_AND:
        return "AND";
    case OP_OR:
        return "OR";
    case OP_NOT:
        return "NOT";
    case OP_CONCAT:
        return "&";
    default:
        return "UNKNOWN";
    }
}
