#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "runtime.h"

/* Expression evaluation functions */

double eval_numeric_expr(RuntimeState *state, ASTExpr *expr);
char *eval_string_expr(RuntimeState *state, ASTExpr *expr);
int eval_condition(RuntimeState *state, ASTExpr *expr);

int eval_is_true(double value);

/* Helper functions */
int is_string_variable(const char *name);
int is_string_expr(ASTExpr *expr);

#endif /* EVAL_H */
