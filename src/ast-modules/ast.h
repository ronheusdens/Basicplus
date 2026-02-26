
/* Video memory backend (CoCo graphics/color) */
#include "video_backend.h"
#ifndef AST_H
#define AST_H

#include "common.h"

/*
 * Statement type enumeration
 */
typedef enum
{
    STMT_PRINT,
    STMT_PRINT_AT,
    STMT_PRINT_USING,
    STMT_INPUT,
    STMT_LINE_INPUT,
    STMT_LET,
    STMT_IF,
    STMT_ON_GOTO,
    STMT_FOR,
    STMT_NEXT,
    STMT_GOTO,
    STMT_GOSUB,
    STMT_RETURN,
    STMT_READ,
    STMT_DATA,
    STMT_RESTORE,

    /*
     * ast_eval_expr prototype moved here for correct ordering
     */
    STMT_DIM,
    STMT_OPEN,
    STMT_CLOSE,
    STMT_WRITE,
    STMT_GET,
    STMT_PUT,
    STMT_END,
    STMT_REM,
    STMT_BLOCK,
    STMT_DEFINT,
    STMT_DEFSNG,
    STMT_DEFDBL,
    STMT_DEFSTR,
    STMT_ON_ERROR,
    STMT_RESUME,
    STMT_ERROR,
    STMT_SLEEP,
    STMT_BEEP,
    STMT_CLS,
    STMT_RANDOMIZE,
    STMT_POKE,
    STMT_CALL,
    STMT_DEF_FN,
    STMT_TRON,
    STMT_TROFF,
    STMT_WHILE,
    STMT_WEND,
    STMT_DO_LOOP,
    STMT_EXIT,
    STMT_SAVE,
    STMT_DELETE,
    STMT_MERGE,
    STMT_CLEAR,
    STMT_COLOR,
    STMT_PCOLOR,
    STMT_SET,
    STMT_RESET,
    STMT_LINE,
    STMT_CIRCLE,
    STMT_PAINT,
    STMT_SCREEN,
    STMT_CASE,
    STMT_STOP,
    STMT_CONT,
    STMT_SOUND,
    STMT_UNKNOWN
} StmtType;

/*
 * Expression type enumeration
 */
typedef enum
{
    EXPR_NUMBER,
    EXPR_STRING,
    EXPR_PRINT_SEP,
    EXPR_TAB,
    EXPR_VAR,
    EXPR_ARRAY,
    EXPR_BINARY_OP,
    EXPR_UNARY_OP,
    EXPR_FUNC_CALL,
    /* Legacy aliases for compatibility */
    EXPR_VARIABLE = EXPR_VAR,
    EXPR_ARRAY_ACCESS = EXPR_ARRAY,
    EXPR_FUNCTION_CALL = EXPR_FUNC_CALL,
    EXPR_CAST,
    EXPR_UNKNOWN
} ExprType;

/*
 * Operator type enumeration
 */
typedef enum
{
    OP_NONE = 0,
    OP_PLUS,
    OP_NEG,            /* Unary minus */
    OP_ADD,            /* Binary add */
    OP_SUB,            /* Binary subtract */
    OP_MINUS = OP_SUB, /* Alias */
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POWER,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_CONCAT,
    OP_UNKNOWN
} OpType;

/* Forward declarations */
typedef struct ASTExpr ASTExpr;
typedef struct ASTStmt ASTStmt;

double ast_eval_expr(ASTExpr *expr);

/* ast_eval_expr prototype moved here for correct ordering */
double ast_eval_expr(ASTExpr *expr);

int ast_execute_stmt(ASTStmt *stmt);

/*
 * Expression Node - Simplified structure
 */
struct ASTExpr
{
    ExprType type;
    int line_number;
    int column_number;
    VarType inferred_type;

    /* Direct fields for simple implementation */
    double num_value; /* For EXPR_NUMBER */
    char *str_value;  /* For EXPR_STRING */
    char *var_name;   /* For EXPR_VAR, EXPR_ARRAY, EXPR_FUNC_CALL */
    OpType op;        /* For EXPR_BINARY_OP, EXPR_UNARY_OP */

    /* Child expressions */
    ASTExpr **children;
    int num_children;
    int capacity_children;
};

/*
 * Statement Node
 */
struct ASTStmt
{
    StmtType type;
    int line_number;

    /* General-purpose fields for simple implementation */
    ASTExpr **exprs; /* Expressions (print args, input vars, etc.) */
    int num_exprs;
    int capacity_exprs;

    ASTStmt *body;      /* Body statement (for IF, FOR, etc.) */
    ASTStmt *else_body; /* ELSE branch for IF */
    ASTStmt *next;      /* Next statement in same line (colon-separated) */
    int target_line;    /* Target line number (for GOTO, GOSUB) */
    int file_handle;    /* File handle for file I/O statements */
    int mode;           /* File open mode or flags */
    char *comment;      /* Comment text (for REM) */
    char *var_name;     /* Variable name (for LET, FOR, DIM, INPUT) */

    /* DO..LOOP specific fields */
    int is_loop_end; /* 0=DO statement, 1=LOOP statement */
    struct
    {
        int condition_type; /* 0=none, 1=pre-test WHILE, 2=post-test WHILE, 3=post-test UNTIL */
    } data;
};

/*
 * Program line (single statement per line)
 */
typedef struct
{
    int line_number;
    ASTStmt *stmt;
} ProgramLine;

/*
 * Program (collection of lines)
 */
typedef struct
{
    ProgramLine **lines;
    int num_lines;
    int capacity;
} Program;

/* AST creation and manipulation functions */

ASTExpr *ast_expr_create(ExprType type);
ASTStmt *ast_stmt_create(StmtType type);
Program *ast_program_create(void);

void ast_expr_free(ASTExpr *expr);
ASTExpr *ast_expr_copy(ASTExpr *expr);
void ast_stmt_free(ASTStmt *stmt);
void ast_program_free(Program *prog);

void ast_expr_print(ASTExpr *expr);
void ast_stmt_print(ASTStmt *stmt);
void ast_program_print(Program *prog);

/* Helper functions for building AST */
void ast_expr_add_child(ASTExpr *expr, ASTExpr *child);
void ast_stmt_add_expr(ASTStmt *stmt, ASTExpr *expr);
void ast_stmt_set_body(ASTStmt *stmt, ASTStmt *body);
void ast_program_add_line(Program *prog, ProgramLine *line);
ProgramLine *ast_program_line_create(int line_num, ASTStmt *stmt);

const char *stmt_type_name(StmtType type);
const char *expr_type_name(ExprType type);
const char *op_type_name(OpType type);

/* CASE statement helper structure (internal use)*/
typedef struct
{
    ASTExpr *value;  /* Value to match */
    ASTStmt **stmts; /* Statements in this branch */
    int stmt_count;
} CaseBranch;

#endif /* AST_H */
