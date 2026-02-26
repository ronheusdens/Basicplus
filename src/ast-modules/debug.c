#include "debug.h"

DebugState *debug_create(void)
{
    DebugState *state = xcalloc(1, sizeof(DebugState));
    state->enabled = 0;
    state->breakpoint_line = -1;
    state->trace_enabled = 0;
    return state;
}

void debug_free(DebugState *state)
{
    free(state);
}

void debug_print_state(DebugState *state, RuntimeState *runtime)
{
    /* Stub: debug printing will be implemented in Phase 4 */
}

void debug_print_variable(RuntimeState *runtime, const char *var_name)
{
    /* Stub: variable printing will be implemented in Phase 4 */
}
