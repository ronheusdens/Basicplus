#include "ast.h"

/* Helper functions for building AST */

void ast_expr_add_child(ASTExpr *expr, ASTExpr *child)
{
    if (expr == NULL || child == NULL)
    {
        return;
    }

    if (expr->num_children >= expr->capacity_children)
    {
        int new_capacity = (expr->capacity_children == 0) ? 4 : expr->capacity_children * 2;
        expr->children = xrealloc(expr->children, new_capacity * sizeof(ASTExpr *));
        expr->capacity_children = new_capacity;
    }

    expr->children[expr->num_children++] = child;
}

void ast_stmt_add_expr(ASTStmt *stmt, ASTExpr *expr)
{
    if (stmt == NULL || expr == NULL)
    {
        return;
    }

    if (stmt->num_exprs >= stmt->capacity_exprs)
    {
        int new_capacity = (stmt->capacity_exprs == 0) ? 4 : stmt->capacity_exprs * 2;
        stmt->exprs = xrealloc(stmt->exprs, new_capacity * sizeof(ASTExpr *));
        stmt->capacity_exprs = new_capacity;
    }

    stmt->exprs[stmt->num_exprs++] = expr;
}

void ast_stmt_set_body(ASTStmt *stmt, ASTStmt *body)
{
    if (stmt != NULL)
    {
        stmt->body = body;
    }
}

void ast_program_add_line(Program *prog, ProgramLine *line)
{
    if (prog == NULL || line == NULL)
    {
        return;
    }

    if (prog->num_lines >= prog->capacity)
    {
        prog->capacity *= 2;
        prog->lines = xrealloc(prog->lines, prog->capacity * sizeof(ProgramLine *));
    }

    prog->lines[prog->num_lines++] = line;
}

ProgramLine *ast_program_line_create(int line_num, ASTStmt *stmt)
{
    ProgramLine *line = xcalloc(1, sizeof(ProgramLine));
    line->line_number = line_num;
    line->stmt = stmt;
    return line;
}
