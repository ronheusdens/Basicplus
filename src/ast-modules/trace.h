#ifndef TRACE_H
#define TRACE_H

#include "ast.h"
#include "runtime.h"

/* Forward declaration - ExecutionContext is defined in executor.h */
struct ExecutionContext;

typedef struct
{
    int enabled;          /* TRON: enabled tracing */
    int step_mode;        /* STEP: execute one line then pause */
    int breakpoints[256]; /* Line numbers with breakpoints */
    int breakpoint_count;
    int break_on_next; /* Internal: break after current statement */
} TraceState;

/* Global trace state */
extern TraceState g_trace_state;

/* Initialize trace system */
void trace_init(void);

/* Control commands */
void trace_enable(void);   /* TRON */
void trace_disable(void);  /* TROFF */
void trace_step(void);     /* STEP */
void trace_continue(void); /* CONT */

/* Breakpoint management */
int trace_add_breakpoint(int line_number);
int trace_remove_breakpoint(int line_number);
int trace_has_breakpoint(int line_number);
void trace_clear_breakpoints(void);
void trace_list_breakpoints(void);

/* Tracing execution */
void trace_before_statement(struct ExecutionContext *ctx, int line_number, ASTStmt *stmt);

/* Variable inspection */
void trace_print_variables(RuntimeState *runtime);
void trace_print_variable(RuntimeState *runtime, const char *name);

#endif /* TRACE_H */
