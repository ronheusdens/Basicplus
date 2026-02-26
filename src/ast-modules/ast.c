#include "ast.h"

#include "video_backend.h"

extern struct VideoMemory video_memory;

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
    expr->num_value = 0.0;
    expr->str_value = NULL;
    expr->var_name = NULL;
    expr->op = OP_NONE;
    expr->children = NULL;
    expr->num_children = 0;
    expr->capacity_children = 0;
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
    stmt->exprs = NULL;
    stmt->num_exprs = 0;
    stmt->capacity_exprs = 0;
    stmt->body = NULL;
    stmt->else_body = NULL;
    stmt->next = NULL;
    stmt->target_line = 0;
    stmt->file_handle = 0;
    stmt->mode = 0;
    stmt->comment = NULL;
    stmt->var_name = NULL;
    return stmt;
}

/*
 * Program creation
 */

Program *ast_program_create(void)
{
    Program *prog = xcalloc(1, sizeof(Program));
    prog->capacity = 1024;
    prog->lines = xmalloc(prog->capacity * sizeof(ProgramLine *));
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

    /* Free string fields */
    if (expr->str_value != NULL)
    {
        free(expr->str_value);
    }
    if (expr->var_name != NULL)
    {
        free(expr->var_name);
    }

    /* Free children */
    if (expr->children != NULL)
    {
        for (int i = 0; i < expr->num_children; i++)
        {
            ast_expr_free(expr->children[i]);
        }
        free(expr->children);
    }

    free(expr);
}

/*
 * Expression deep copy (recursive)
 */
ASTExpr *ast_expr_copy(ASTExpr *expr)
{
    if (expr == NULL)
    {
        return NULL;
    }

    ASTExpr *copy = ast_expr_create(expr->type);
    copy->line_number = expr->line_number;
    copy->column_number = expr->column_number;
    copy->inferred_type = expr->inferred_type;
    copy->num_value = expr->num_value;
    copy->op = expr->op;

    /* Copy string fields */
    if (expr->str_value != NULL)
    {
        copy->str_value = xstrdup(expr->str_value);
    }
    if (expr->var_name != NULL)
    {
        copy->var_name = xstrdup(expr->var_name);
    }

    /* Recursively copy children */
    for (int i = 0; i < expr->num_children; i++)
    {
        ast_expr_add_child(copy, ast_expr_copy(expr->children[i]));
    }

    return copy;
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

    /* Free expressions */
    if (stmt->exprs != NULL)
    {
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            ast_expr_free(stmt->exprs[i]);
        }
        free(stmt->exprs);
    }

    /* Free body recursively */
    if (stmt->body != NULL)
    {
        ast_stmt_free(stmt->body);
    }

    /* Free else body recursively */
    if (stmt->else_body != NULL)
    {
        ast_stmt_free(stmt->else_body);
    }

    /* Free chained statements */
    if (stmt->next != NULL)
    {
        ast_stmt_free(stmt->next);
    }

    /* Free comment */
    if (stmt->comment != NULL)
    {
        free(stmt->comment);
    }

    /* Free var_name */
    if (stmt->var_name != NULL)
    {
        free(stmt->var_name);
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
            if (prog->lines[i] != NULL)
            {
                ast_stmt_free(prog->lines[i]->stmt);
                free(prog->lines[i]);
            }
        }
        free(prog->lines);
    }

    free(prog);
}

/*
 * Printing functions
 */

static void print_indent(int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("  ");
    }
}

void ast_expr_print(ASTExpr *expr)
{
    if (expr == NULL)
    {
        printf("(null)");
        return;
    }

    switch (expr->type)
    {
    case EXPR_NUMBER:
        printf("%.2f", expr->num_value);
        break;
    case EXPR_STRING:
        printf("\"%s\"", expr->str_value ? expr->str_value : "");
        break;
    case EXPR_VAR:
        printf("%s", expr->var_name ? expr->var_name : "(unnamed)");
        break;
    case EXPR_ARRAY:
        printf("%s[", expr->var_name ? expr->var_name : "(unnamed)");
        for (int i = 0; i < expr->num_children; i++)
        {
            if (i > 0)
                printf(", ");
            ast_expr_print(expr->children[i]);
        }
        printf("]");
        break;
    case EXPR_FUNC_CALL:
        printf("%s(", expr->var_name ? expr->var_name : "(unnamed)");
        for (int i = 0; i < expr->num_children; i++)
        {
            if (i > 0)
                printf(", ");
            ast_expr_print(expr->children[i]);
        }
        printf(")");
        break;
    case EXPR_BINARY_OP:
        printf("(");
        if (expr->num_children >= 1)
            ast_expr_print(expr->children[0]);
        printf(" %s ", op_type_name(expr->op));
        if (expr->num_children >= 2)
            ast_expr_print(expr->children[1]);
        printf(")");
        break;
    case EXPR_UNARY_OP:
        printf("(%s ", op_type_name(expr->op));
        if (expr->num_children >= 1)
            ast_expr_print(expr->children[0]);
        printf(")");
        break;
    default:
        printf("(unknown expr)");
        break;
    }
}

void ast_stmt_print(ASTStmt *stmt);
// AST interpreter execution stub for CoCo graphics/color statements
int ast_execute_stmt(ASTStmt *stmt)
{
    if (!stmt)
        return 0;
    switch (stmt->type)
    {
    case STMT_COLOR:
    {
        int fg = 1, bg = 0;
        if (stmt->num_exprs > 0)
            fg = (int)ast_eval_expr(stmt->exprs[0]);
        if (stmt->num_exprs > 1)
            bg = (int)ast_eval_expr(stmt->exprs[1]);
        video_set_color(fg);
        video_set_background(bg);
        return 0;
    }
    case STMT_PCOLOR:
    {
        int palette = 0;
        if (stmt->num_exprs > 0)
            palette = (int)ast_eval_expr(stmt->exprs[0]);
        // TODO: video_set_palette(palette);
        return 0;
    }
    case STMT_SET:
    {
        int x = 0, y = 0, color = video_memory.current_color;
        if (stmt->num_exprs > 0)
            x = (int)ast_eval_expr(stmt->exprs[0]);
        if (stmt->num_exprs > 1)
            y = (int)ast_eval_expr(stmt->exprs[1]);
        if (stmt->num_exprs > 2)
            color = (int)ast_eval_expr(stmt->exprs[2]);
        video_set_pixel(x, y, color);
        return 0;
    }
    case STMT_RESET:
    {
        int x = 0, y = 0;
        if (stmt->num_exprs > 0)
            x = (int)ast_eval_expr(stmt->exprs[0]);
        if (stmt->num_exprs > 1)
            y = (int)ast_eval_expr(stmt->exprs[1]);
        video_set_pixel(x, y, video_memory.background_color);
        return 0;
    }
    case STMT_LINE:
    {
        int color = video_memory.current_color;
        int n = stmt->num_exprs;
        if (n >= 4)
        {
            if (n == 5)
            {
                int x1 = (int)ast_eval_expr(stmt->exprs[0]);
                int y1 = (int)ast_eval_expr(stmt->exprs[1]);
                int x2 = (int)ast_eval_expr(stmt->exprs[2]);
                int y2 = (int)ast_eval_expr(stmt->exprs[3]);
                color = (int)ast_eval_expr(stmt->exprs[4]);
                video_draw_line(x1, y1, x2, y2, color);
            }
            else if (n >= 6 && (n % 2) == 0)
            {
                int num_points = n / 2;
                for (int i = 0; i < num_points; i++)
                {
                    int i0 = i * 2;
                    int i1 = i0 + 1;
                    int i2 = ((i + 1) % num_points) * 2;
                    int i3 = i2 + 1;
                    int x1 = (int)ast_eval_expr(stmt->exprs[i0]);
                    int y1 = (int)ast_eval_expr(stmt->exprs[i1]);
                    int x2 = (int)ast_eval_expr(stmt->exprs[i2]);
                    int y2 = (int)ast_eval_expr(stmt->exprs[i3]);
                    video_draw_line(x1, y1, x2, y2, color);
                }
            }
            else if (n == 4)
            {
                int x1 = (int)ast_eval_expr(stmt->exprs[0]);
                int y1 = (int)ast_eval_expr(stmt->exprs[1]);
                int x2 = (int)ast_eval_expr(stmt->exprs[2]);
                int y2 = (int)ast_eval_expr(stmt->exprs[3]);
                video_draw_line(x1, y1, x2, y2, color);
            }
        }
        return 0;
    }
    case STMT_CIRCLE:
    {
        int x = 0, y = 0, radius = 0, color = video_memory.current_color;
        if (stmt->num_exprs > 0)
            x = (int)ast_eval_expr(stmt->exprs[0]);
        if (stmt->num_exprs > 1)
            y = (int)ast_eval_expr(stmt->exprs[1]);
        if (stmt->num_exprs > 2)
            radius = (int)ast_eval_expr(stmt->exprs[2]);
        if (stmt->num_exprs > 3)
            color = (int)ast_eval_expr(stmt->exprs[3]);
        video_draw_circle(x, y, radius, color);
        return 0;
    }
    case STMT_PAINT:
    {
        int x = 0, y = 0, color = video_memory.current_color;
        if (stmt->num_exprs > 0)
            x = (int)ast_eval_expr(stmt->exprs[0]);
        if (stmt->num_exprs > 1)
            y = (int)ast_eval_expr(stmt->exprs[1]);
        if (stmt->num_exprs > 2)
            color = (int)ast_eval_expr(stmt->exprs[2]);
        // TODO: video_paint(x, y, color);
        return 0;
    }
    case STMT_SCREEN:
    {
        int mode = 0;
        if (stmt->num_exprs > 0)
            mode = (int)ast_eval_expr(stmt->exprs[0]);
        video_set_screen_mode(mode);
        return 0;
    }
    default:
        return 0;
    }
}
void ast_stmt_print(ASTStmt *stmt)
{
    if (stmt == NULL)
    {
        return;
    }

    printf("%s", stmt_type_name(stmt->type));

    if (stmt->type == STMT_GOTO || stmt->type == STMT_GOSUB)
    {
        printf(" %d", stmt->target_line);
    }
    else if (stmt->type == STMT_REM && stmt->comment)
    {
        printf(" %s", stmt->comment);
    }
    else if (stmt->num_exprs > 0)
    {
        printf(" ");
        for (int i = 0; i < stmt->num_exprs; i++)
        {
            if (i > 0)
                printf(", ");
            ast_expr_print(stmt->exprs[i]);
        }
    }

    if (stmt->body)
    {
        printf(" THEN ");
        ast_stmt_print(stmt->body);
    }
}

void ast_program_print(Program *prog)
{
    if (prog == NULL)
    {
        return;
    }

    for (int i = 0; i < prog->num_lines; i++)
    {
        if (prog->lines[i])
        {
            printf("%d ", prog->lines[i]->line_number);
            ast_stmt_print(prog->lines[i]->stmt);
            printf("\n");
        }
    }
}

/*
 * Name functions
 */

const char *stmt_type_name(StmtType type)
{
    switch (type)
    {
    case STMT_PRINT:
        return "PRINT";
    case STMT_LET:
        return "LET";
    case STMT_INPUT:
        return "INPUT";
    case STMT_IF:
        return "IF";
    case STMT_GOTO:
        return "GOTO";
    case STMT_GOSUB:
        return "GOSUB";
    case STMT_RETURN:
        return "RETURN";
    case STMT_FOR:
        return "FOR";
    case STMT_NEXT:
        return "NEXT";
    case STMT_READ:
        return "READ";
    case STMT_DATA:
        return "DATA";
    case STMT_DIM:
        return "DIM";
    case STMT_END:
        return "END";
    case STMT_REM:
        return "REM";
    case STMT_ON_ERROR:
        return "ON_ERROR";
    case STMT_RESUME:
        return "RESUME";
    case STMT_ERROR:
        return "ERROR";
    case STMT_DEFINT:
        return "DEFINT";
    case STMT_DEFSNG:
        return "DEFSNG";
    case STMT_DEFDBL:
        return "DEFDBL";
    case STMT_DEFSTR:
        return "DEFSTR";
    case STMT_COLOR:
        return "COLOR";
    case STMT_PCOLOR:
        return "PCOLOR";
    case STMT_SET:
        return "SET";
    case STMT_RESET:
        return "RESET";
    case STMT_LINE:
        return "LINE";
    case STMT_CIRCLE:
        return "CIRCLE";
    case STMT_PAINT:
        return "PAINT";
    case STMT_SCREEN:
        return "SCREEN";
    case STMT_STOP:
        return "STOP";
    case STMT_CONT:
        return "CONT";
    case STMT_SOUND:
        return "SOUND";
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
    case EXPR_PRINT_SEP:
        return "PRINT_SEP";
    case EXPR_VAR:
        return "VAR";
    case EXPR_ARRAY:
        return "ARRAY";
    case EXPR_BINARY_OP:
        return "BINARY_OP";
    case EXPR_UNARY_OP:
        return "UNARY_OP";
    case EXPR_FUNC_CALL:
        return "FUNC_CALL";
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
    case OP_NEG:
        return "-";
    case OP_ADD:
        return "+";
    case OP_SUB:
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
