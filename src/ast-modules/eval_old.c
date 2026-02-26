#include "eval.h"

double eval_numeric_expr(RuntimeState *state, ASTExpr *expr)
{
    /* Stub: numeric expression evaluation will be implemented in Phase 3 */
    if (expr == NULL)
    {
        return 0.0;
    }
    return 0.0;
}

char *eval_string_expr(RuntimeState *state, ASTExpr *expr)
{
    /* Stub: string expression evaluation will be implemented in Phase 3 */
    if (expr == NULL)
    {
        return "";
    }
    return "";
}

int eval_condition(RuntimeState *state, ASTExpr *expr)
{
    /* Stub: condition evaluation will be implemented in Phase 3 */
    return 0;
}

int eval_is_true(double value)
{
    return value != 0.0;
}
