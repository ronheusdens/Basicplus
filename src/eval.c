#include "eval.h"
#include "executor.h"
#include "builtins.h"
#include "errors.h"
#include <string.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>

/* Forward declarations */
static double eval_expr_internal(RuntimeState *state, ASTExpr *expr);
static char *eval_string_expr_internal(RuntimeState *state, ASTExpr *expr);

static int eval_expr_is_string(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0;
    }

    if (expr->type == EXPR_STRING)
    {
        return 1;
    }

    if (expr->type == EXPR_VAR || expr->type == EXPR_ARRAY || expr->type == EXPR_FUNC_CALL)
    {
        if (expr->var_name && expr->var_name[0] != '\0')
        {
            size_t len = strlen(expr->var_name);
            if (expr->var_name[len - 1] == '$')
            {
                return 1;
            }
        }

        if (expr->type == EXPR_VAR || expr->type == EXPR_ARRAY)
        {
            if (expr->var_name && runtime_get_variable_type(state, expr->var_name) == VAR_STRING)
            {
                return 1;
            }
        }
    }

    if (expr->type == EXPR_BINARY_OP && expr->op == OP_CONCAT)
    {
        return 1;
    }

    return 0;
}

double eval_numeric_expr(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0.0;
    }
    return eval_expr_internal(state, expr);
}

char *eval_string_expr(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return xstrdup("");
    }
    return eval_string_expr_internal(state, expr);
}

int eval_condition(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0;
    }
    double result = eval_expr_internal(state, expr);
    return eval_is_true(result);
}

int eval_is_true(double value)
{
    return value != 0.0;
}

double ast_eval_expr(ASTExpr *expr)
{
    RuntimeState *state = runtime_get_current_state();
    if (state == NULL || expr == NULL)
    {
        return 0.0;
    }
    return eval_numeric_expr(state, expr);
}

static double eval_expr_internal(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0.0;
    }

    switch (expr->type)
    {
    case EXPR_NUMBER:
        return expr->num_value;

    case EXPR_STRING:
        /* Convert string to number if needed */
        return expr->str_value ? atof(expr->str_value) : 0.0;

    case EXPR_PRINT_SEP:
        return 0.0;

    case EXPR_VAR:
        /* Variable reference */
        if (expr->var_name)
        {
            if (strcasecmp(expr->var_name, "ERR") == 0)
            {
                return (double)runtime_get_error(state);
            }
            if (strcasecmp(expr->var_name, "ERL") == 0)
            {
                return (double)runtime_get_error_line(state);
            }
            return runtime_get_variable(state, expr->var_name);
        }
        return 0.0;

    case EXPR_ARRAY:
        /* Array element access */
        if (expr->var_name && expr->num_children > 0)
        {
            /* Evaluate indices */
            int *indices = xmalloc(expr->num_children * sizeof(int));
            for (int i = 0; i < expr->num_children; i++)
            {
                indices[i] = (int)eval_expr_internal(state, expr->children[i]);
            }
            double result = runtime_get_array_element(state, expr->var_name,
                                                      indices, expr->num_children);
            free(indices);
            return result;
        }
        return 0.0;

    case EXPR_BINARY_OP:
        if (expr->num_children >= 2)
        {
            ASTExpr *left_expr = expr->children[0];
            ASTExpr *right_expr = expr->children[1];
            int use_string_compare = eval_expr_is_string(state, left_expr) || eval_expr_is_string(state, right_expr);

            double left = eval_expr_internal(state, left_expr);
            double right = eval_expr_internal(state, right_expr);
            char *left_str = NULL;
            char *right_str = NULL;

            if (use_string_compare)
            {
                left_str = eval_string_expr_internal(state, left_expr);
                right_str = eval_string_expr_internal(state, right_expr);
            }

            switch (expr->op)
            {
            case OP_ADD:
                return left + right;
            case OP_SUB:
                return left - right;
            case OP_MUL:
                return left * right;
            case OP_DIV:
                if (right == 0.0)
                {
                    runtime_set_error(state, BASIC_ERR_DIVISION_BY_ZERO, expr->line_number);
                    return 0.0;
                }
                return left / right;
            case OP_MOD:
                return (right != 0.0) ? fmod(left, right) : 0.0;
            case OP_POWER:
                return pow(left, right);
            case OP_EQ:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp == 0) ? -1.0 : 0.0; /* BASIC true = -1 */
                }
                return (fabs(left - right) < 1e-9) ? -1.0 : 0.0; /* BASIC true = -1 */
            case OP_NE:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp != 0) ? -1.0 : 0.0;
                }
                return (fabs(left - right) >= 1e-9) ? -1.0 : 0.0;
            case OP_LT:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp < 0) ? -1.0 : 0.0;
                }
                return (left < right) ? -1.0 : 0.0;
            case OP_LE:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp <= 0) ? -1.0 : 0.0;
                }
                return (left <= right) ? -1.0 : 0.0;
            case OP_GT:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp > 0) ? -1.0 : 0.0;
                }
                return (left > right) ? -1.0 : 0.0;
            case OP_GE:
                if (use_string_compare)
                {
                    int cmp = strcmp(left_str ? left_str : "", right_str ? right_str : "");
                    if (left_str)
                        free(left_str);
                    if (right_str)
                        free(right_str);
                    return (cmp >= 0) ? -1.0 : 0.0;
                }
                return (left >= right) ? -1.0 : 0.0;
            case OP_AND:
                return (eval_is_true(left) && eval_is_true(right)) ? -1.0 : 0.0;
            case OP_OR:
                return (eval_is_true(left) || eval_is_true(right)) ? -1.0 : 0.0;
            default:
                if (left_str)
                    free(left_str);
                if (right_str)
                    free(right_str);
                return 0.0;
            }
        }
        return 0.0;

    case EXPR_UNARY_OP:
        if (expr->num_children >= 1)
        {
            double operand = eval_expr_internal(state, expr->children[0]);

            switch (expr->op)
            {
            case OP_NEG:
            case OP_MINUS:
                return -operand;
            case OP_PLUS:
                return operand;
            case OP_NOT:
                return eval_is_true(operand) ? 0.0 : -1.0;
            default:
                return operand;
            }
        }
        return 0.0;

    case EXPR_FUNC_CALL:
        /* Call built-in function */
        if (expr->var_name)
        {
            return call_numeric_function(state, expr->var_name,
                                         expr->children, expr->num_children);
        }
        return 0.0;

    case EXPR_PROC_CALL:
        /* Call user-defined procedure and return its value */
        if (expr->var_name && state)
        {
            /* Need to have ExecutionContext available (set during execution) */
            void *ctx_ptr = runtime_get_execution_context(state);
            if (ctx_ptr)
            {
                ExecutionContext *ctx = (ExecutionContext *)ctx_ptr;
                return executor_execute_procedure_expr(ctx, expr->var_name,
                                                       expr->children, expr->num_children);
            }
        }
        return 0.0;

    case EXPR_MEMBER_ACCESS:
        /* obj.method(...) or obj.field - member access */
        if (expr->member_obj && expr->member_name && state)
        {
            /* Evaluate member_obj to get instance ID */
            double obj_id_val = ast_eval_expr(expr->member_obj);
            int obj_id = (int)obj_id_val;

            void *ctx_ptr = runtime_get_execution_context(state);
            if (ctx_ptr)
            {
                ExecutionContext *ctx = (ExecutionContext *)ctx_ptr;
                ObjectInstance *instance = runtime_get_instance(ctx->runtime, obj_id);

                if (instance)
                {
                    /* Check if this is a method call (has children = arguments) */
                    if (expr->num_children >= 0) /* Method call (0 or more args) */
                    {
                        ClassDef *class_def = runtime_lookup_class(ctx->runtime, instance->class_name);
                        if (class_def)
                        {
                            /* Prepend object as first argument for method call */
                            ASTExpr **method_args = xmalloc(sizeof(ASTExpr *) * (expr->num_children + 1));
                            method_args[0] = expr->member_obj; /* Object is first arg */
                            for (int i = 0; i < expr->num_children; i++)
                            {
                                method_args[i + 1] = expr->children[i];
                            }

                            double result = executor_execute_procedure_expr(ctx, expr->member_name,
                                                                            method_args, expr->num_children + 1);
                            free(method_args);
                            return result;
                        }
                    }
                    else
                    {
                        /* Field access */
                        return runtime_get_instance_variable(instance, expr->member_name);
                    }
                }
            }
        }
        return 0.0;

    case EXPR_NEW:
        /* NEW ClassName(...) - create new instance */
        if (getenv("AST_DEBUG"))
        {
            fprintf(stderr, "[EVAL] EXPR_NEW: var_name=%s, state=%p\n",
                    expr->var_name ? expr->var_name : "NULL", (void *)state);
        }
        if (expr->var_name && state)
        {
            void *ctx_ptr = runtime_get_execution_context(state);
            if (getenv("AST_DEBUG"))
            {
                fprintf(stderr, "[EVAL] EXPR_NEW: ctx_ptr=%p\n", ctx_ptr);
            }
            if (ctx_ptr)
            {
                ExecutionContext *ctx = (ExecutionContext *)ctx_ptr;
                ObjectInstance *inst = runtime_create_instance(ctx->runtime, expr->var_name);
                if (inst)
                {
                    /* Bind constructor arguments to instance variables */
                    ClassDef *class_def = runtime_lookup_class(ctx->runtime, expr->var_name);
                    if (class_def && class_def->parameters)
                    {
                        ASTParameterList *params = (ASTParameterList *)class_def->parameters;
                        /* Bind each constructor argument to parameter name */
                        for (int i = 0; i < params->num_params && i < expr->num_children; i++)
                        {
                            ASTParameter *param = params->params[i];
                            ASTExpr *arg = expr->children[i];
                            if (param && arg && param->name)
                            {
                                double arg_value = ast_eval_expr(arg);
                                runtime_set_instance_variable(inst, param->name, arg_value);
                                if (getenv("AST_DEBUG"))
                                {
                                    fprintf(stderr, "[NEW] Bound %s = %g\n", param->name, arg_value);
                                }
                            }
                        }
                    }

                    if (getenv("AST_DEBUG"))
                    {
                        fprintf(stderr, "[NEW] Created instance of %s with ID %d\n",
                                expr->var_name, inst->instance_id);
                    }
                    return (double)inst->instance_id;
                }
                else
                {
                    if (getenv("AST_DEBUG"))
                    {
                        fprintf(stderr, "[NEW] Failed to create instance of %s\n", expr->var_name);
                    }
                }
            }
        }
        return 0.0;

    default:
        return 0.0;
    }
}

static char *eval_string_expr_internal(RuntimeState *state, ASTExpr *expr)
{
    if (expr == NULL)
    {
        return xstrdup("");
    }

    switch (expr->type)
    {
    case EXPR_STRING:
        return xstrdup(expr->str_value ? expr->str_value : "");

    case EXPR_NUMBER:
    {
        char buf[64];
        if (fabs(expr->num_value) < 1e-10 && expr->num_value != 0.0)
            snprintf(buf, sizeof(buf), "%.9e", expr->num_value);
        else
            snprintf(buf, sizeof(buf), "%.15g", expr->num_value);
        return xstrdup(buf);
    }

    case EXPR_VAR:
        /* Variable reference */
        if (expr->var_name)
        {
            return runtime_get_string_variable(state, expr->var_name);
        }
        return xstrdup("");

    case EXPR_FUNC_CALL:
        if (expr->var_name)
        {
            return call_string_function(state, expr->var_name, expr->children, expr->num_children);
        }
        return xstrdup("");

    case EXPR_PROC_CALL:
        /* Call procedure that returns numeric value, convert to string */
        if (expr->var_name && state)
        {
            void *ctx_ptr = runtime_get_execution_context(state);
            if (ctx_ptr)
            {
                ExecutionContext *ctx = (ExecutionContext *)ctx_ptr;
                double result = executor_execute_procedure_expr(ctx, expr->var_name,
                                                                expr->children, expr->num_children);
                char buf[64];
                if (fabs(result) < 1e-10 && result != 0.0)
                    snprintf(buf, sizeof(buf), "%.9e", result);
                else
                    snprintf(buf, sizeof(buf), "%.15g", result);
                return xstrdup(buf);
            }
        }
        return xstrdup("");

    case EXPR_ARRAY:
        if (expr->var_name && expr->num_children > 0)
        {
            int *indices = xmalloc(expr->num_children * sizeof(int));
            for (int i = 0; i < expr->num_children; i++)
            {
                indices[i] = (int)eval_expr_internal(state, expr->children[i]);
            }
            char *result = runtime_get_string_array_element(state, expr->var_name, indices, expr->num_children);
            free(indices);
            return result;
        }
        return xstrdup("");

    case EXPR_BINARY_OP:
        /* String concatenation */
        if (expr->op == OP_CONCAT && expr->num_children >= 2)
        {
            /* Type check: both operands must be strings */
            if (!is_string_expr(expr->children[0]) || !is_string_expr(expr->children[1]))
            {
                runtime_set_error(state, BASIC_ERR_TYPE_MISMATCH, 0);
                return xstrdup("");
            }

            char *left = eval_string_expr_internal(state, expr->children[0]);
            char *right = eval_string_expr_internal(state, expr->children[1]);
            size_t len = strlen(left) + strlen(right) + 1;
            char *result = xmalloc(len);
            strcpy(result, left);
            strcat(result, right);
            free(left);
            free(right);
            return result;
        }
        else if (expr->num_children >= 2)
        {
            /* Convert to numeric and back */
            double num = eval_expr_internal(state, expr);
            char buf[64];
            if (fabs(num) < 1e-10 && num != 0.0)
                snprintf(buf, sizeof(buf), "%.9e", num);
            else
                snprintf(buf, sizeof(buf), "%.15g", num);
            return xstrdup(buf);
        }
        return xstrdup("");

    default:
        /* Default: convert to number then to string */
        {
            double num = eval_expr_internal(state, expr);
            char buf[64];
            if (fabs(num) < 1e-10 && num != 0.0)
                snprintf(buf, sizeof(buf), "%.9e", num);
            else
                snprintf(buf, sizeof(buf), "%.15g", num);
            return xstrdup(buf);
        }
    }
}

/* Helper: Check if variable name is string type */
int is_string_variable(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return 0;
    }
    int len = strlen(name);
    return name[len - 1] == '$';
}

/* Helper: Check if expression evaluates to string */
int is_string_expr(ASTExpr *expr)
{
    if (expr == NULL)
    {
        return 0;
    }

    if (expr->type == EXPR_STRING)
    {
        return 1;
    }

    if (expr->type == EXPR_VAR && expr->var_name)
    {
        return is_string_variable(expr->var_name);
    }

    if (expr->type == EXPR_FUNC_CALL && expr->var_name)
    {
        /* String functions: LEFT$, RIGHT$, MID$, CHR$, STR$, etc. */
        int len = strlen(expr->var_name);
        return (len > 0 && expr->var_name[len - 1] == '$');
    }

    if (expr->type == EXPR_BINARY_OP && expr->op == OP_CONCAT)
    {
        /* String concatenation produces a string */
        return 1;
    }

    return 0;
}
