#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "ast.h"
#include "runtime.h"
#include <signal.h>
#include <time.h>

/* Frame structures for FOR and WHILE loops */
typedef struct ForFrame
{
    char *var_name;
    double end;
    double step;
    int for_line_index;
    int next_line_index;
    ASTStmt *body_start;
    ASTStmt *after_next;
} ForFrame;

typedef struct WhileFrame
{
    ASTExpr *condition;
    int while_line_index;
    int wend_line_index;
} WhileFrame;

/* Local variable scope for procedures */
typedef struct
{
    char **var_names;
    double *var_values;
    int *var_existed; /* 1 if variable existed before procedure, 0 if created inside */
    int num_vars;
    int capacity;
} ProcedureScope;

/* Execution context for tracking state during execution */
typedef struct
{
    RuntimeState *runtime;
    Program *program;
    int current_line_index;      /* Index in program->lines */
    int next_line_index;         /* Next line to execute after current */
    ASTStmt *next_stmt_override; /* Next statement to execute on same line */
    int skip_chained;            /* Skip remaining chained statements on current line */
    int return_line_index;       /* For GOSUB/RETURN */
    int error_code;
    char *error_msg;
    ForFrame *for_stack;
    int for_sp;
    int for_cap;
    WhileFrame *while_stack;
    int while_sp;
    int while_cap;
    int proc_return_flag;        /* Set when RETURN executed in procedure */
    double proc_return_value;    /* Return value from procedure */
    int in_procedure;            /* Track if currently in procedure execution */
    ProcedureScope *scope_stack; /* Stack of procedure scopes for local variables */
    int scope_sp;                /* Scope stack pointer */
    int scope_cap;               /* Scope stack capacity */
} ExecutionContext;

/* Executor functions */

int execute_program(RuntimeState *state, Program *prog);
int execute_program_from_line(RuntimeState *state, Program *prog, int start_line_num);
int execute_statement(RuntimeState *state, ASTStmt *stmt, Program *prog);

/* Execute a procedure call in expression context and return its value */
double executor_execute_procedure_expr(ExecutionContext *ctx, const char *proc_name,
                                       ASTExpr **args, int num_args);

void executor_set_interrupt_flag(volatile sig_atomic_t *flag);

int executor_check_interrupt(void);

void executor_trigger_interrupt(void);

void executor_process_events(void);

int find_program_line(Program *prog, int line_number);

/* Accessor for trace support */
RuntimeState *executor_get_runtime(ExecutionContext *ctx);

#endif /* EXECUTOR_H */
