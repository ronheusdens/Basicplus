#ifndef DEBUG_H
#define DEBUG_H

#include "ast.h"
#include "runtime.h"

/*
 * Debugger state
 */
typedef struct
{
    int enabled;
    int breakpoint_line;
    int trace_enabled;
} DebugState;

/* Debug functions */

DebugState *debug_create(void);
void debug_free(DebugState *state);

void debug_print_state(DebugState *state, RuntimeState *runtime);
void debug_print_variable(RuntimeState *runtime, const char *var_name);

#endif /* DEBUG_H */
