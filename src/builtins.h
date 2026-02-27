#ifndef BUILTINS_H
#define BUILTINS_H

#include "ast.h"
#include "runtime.h"

/* Built-in function calls */

double call_numeric_function(RuntimeState *state, const char *func_name,
                                    ASTExpr **args, int num_args);

char *call_string_function(RuntimeState *state, const char *func_name,
                                  ASTExpr **args, int num_args);

#endif /* BUILTINS_H */
