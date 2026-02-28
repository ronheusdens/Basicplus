#include "runtime.h"
#include "symtable.h"
#include "ast.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/* Variable storage using simple dynamic array */
typedef struct
{
    char *name;
    VarType type;
    RuntimeValue value;
    int is_array;
    int *dimensions;
    int num_dimensions;
    int total_elements;
    int address;
} Variable;

typedef struct
{
    VarType type;
    double num_value;
    char *str_value;
} DataValue;

typedef struct
{
    FILE *fp;
    int mode; /* 0 unused, 1 input, 2 output, 3 append */
} FileHandle;

typedef struct
{
    char *name;        /* Function name (e.g., "FNF") */
    char **parameters; /* Parameter names */
    int num_parameters;
    struct ASTExpr *body; /* Function body expression */
} UserDefinedFunction;

struct RuntimeState
{
    /* Variables */
    Variable *variables;
    int num_variables;
    int capacity_variables;

    /* User-defined functions */
    UserDefinedFunction *user_functions;
    int num_user_functions;
    int capacity_user_functions;

    /* Call stack */
    int *call_stack;
    int call_stack_ptr;
    int call_stack_capacity;

    /* Data pointer and values */
    int data_ptr;
    DataValue *data_values;
    int num_data_values;
    int capacity_data_values;
    /* DATA segment boundaries for RESTORE line: (line_number, start_index) */
    int *data_segment_line;
    int *data_segment_start;
    int num_data_segments;
    int capacity_data_segments;

    /* File handles */
    FileHandle *files;
    int max_files;

    /* Memory for POKE/PEEK */
    unsigned char *memory;
    int memory_size;

    /* USR registers */
    int usr_address;
    int reg_a;
    int reg_b;

    /* Error state */
    int error_code;
    int error_line;
    int error_resume_line;
    int error_handler_line;
    int in_error_handler;

    /* DEFxxx defaults */
    VarType letter_types[26];

    /* Random seed and LCG state */
    unsigned int random_seed;
    unsigned int lcg_state; /* Current LCG value */
    double last_rnd_value;  /* Last generated RND value */

    /* Trace flag for TRON/TROFF */
    int trace_on;

    /* Console output pending newline */
    int output_pending;
    int output_col;

    /* Other state */
    int eof_flag;

    /* SAVE callback for program statements */
    SaveCallback save_callback;

    /* DELETE callback for deleting program lines */
    DeleteCallback delete_callback;

    /* MERGE callback for merging program files */
    MergeCallback merge_callback;

    /* Last entered line number (for DELETE .) */
    int last_entered_line;

    /* STOP/CONT state */
    int stopped;
    int stop_line_number;

    /* DO..LOOP stack */
    struct
    {
        int do_line_index;
        int loop_line_index;
        int condition_type; /* 1=pre-test WHILE, 2=post-test WHILE, 3=post-test UNTIL, 0=infinite */
        void *condition;    /* ASTExpr* (void* to avoid circular dependency) */
    } *do_loop_stack;
    int do_loop_sp;
    int do_loop_cap;

    /* Scope stack for procedure local variables */
    ScopeStack *scope_stack;

    /* Procedure registry for storing procedure definitions */
    ProcedureRegistry *procedure_registry;

    /* Class registry for storing class definitions */
    ClassRegistry *class_registry;

    /* Object instances */
    ObjectInstance **instances;
    int num_instances;
    int capacity_instances;
    int next_instance_id;

    /* Execution context - set during statement execution for access from evaluator */
    void *execution_context; /* ExecutionContext* (void* to avoid circular dependency) */
};

/* Helper to find variable by name */
static Variable *find_variable(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < state->num_variables; i++)
    {
        if (strcmp(state->variables[i].name, name) == 0)
        {
            return &state->variables[i];
        }
    }

    return NULL;
}

/* Helper to create variable if not exists */
static Variable *ensure_variable(RuntimeState *state, const char *name, VarType type)
{
    Variable *var = find_variable(state, name);
    if (var != NULL)
    {
        return var;
    }

    /* Expand capacity if needed */
    if (state->num_variables >= state->capacity_variables)
    {
        state->capacity_variables = (state->capacity_variables == 0) ? 256 : state->capacity_variables * 2;
        state->variables = xrealloc(state->variables,
                                    state->capacity_variables * sizeof(Variable));
    }

    /* Create new variable */
    var = &state->variables[state->num_variables++];
    var->name = xstrdup(name);
    var->type = type;
    var->is_array = 0;
    var->dimensions = NULL;
    var->num_dimensions = 0;
    var->total_elements = 0;
    var->address = 1000 + (state->num_variables * 4);

    /* Initialize value based on type */
    if (type == VAR_STRING)
    {
        var->value.str_value = xstrdup("");
    }
    else
    {
        var->value.num_value = 0.0;
    }

    return var;
}

/* Helper to get variable type from name */
static VarType get_var_type_from_name(RuntimeState *state, const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return VAR_DOUBLE;
    }

    int len = strlen(name);
    char last = name[len - 1];

    if (last == '$')
        return VAR_STRING;
    if (last == '%')
        return VAR_INTEGER;
    if (last == '!')
        return VAR_SINGLE;
    if (last == '#')
        return VAR_DOUBLE;

    char first = (char)toupper((unsigned char)name[0]);
    if (state && first >= 'A' && first <= 'Z')
    {
        return state->letter_types[first - 'A'];
    }

    return VAR_DOUBLE; /* Default */
}

void runtime_set_def_range(RuntimeState *state, VarType type, char start_letter, char end_letter)
{
    if (state == NULL)
    {
        return;
    }

    if (start_letter > end_letter)
    {
        return;
    }

    for (char c = start_letter; c <= end_letter; c++)
    {
        if (c >= 'A' && c <= 'Z')
        {
            state->letter_types[c - 'A'] = type;
        }
    }
}

static _Thread_local RuntimeState *g_current_state = NULL;

void runtime_set_current_state(RuntimeState *state)
{
    g_current_state = state;
}

RuntimeState *runtime_get_current_state(void)
{
    return g_current_state;
}

void runtime_set_execution_context(RuntimeState *state, void *ctx)
{
    if (state)
    {
        state->execution_context = ctx;
    }
}

void *runtime_get_execution_context(RuntimeState *state)
{
    if (state)
    {
        return state->execution_context;
    }
    return NULL;
}

RuntimeState *runtime_create(void)
{
    RuntimeState *state = xcalloc(1, sizeof(RuntimeState));

    /* Variables */
    state->capacity_variables = 256;
    state->variables = xmalloc(state->capacity_variables * sizeof(Variable));
    state->num_variables = 0;

    /* User-defined functions */
    state->capacity_user_functions = 64;
    state->user_functions = xmalloc(state->capacity_user_functions * sizeof(UserDefinedFunction));
    state->num_user_functions = 0;

    /* Call stack */
    state->call_stack_capacity = MAX_STACK_DEPTH;
    state->call_stack = xmalloc(state->call_stack_capacity * sizeof(int));
    state->call_stack_ptr = 0;

    /* Error state */
    state->error_code = 0;
    state->error_line = 0;
    state->error_resume_line = 0;
    state->error_handler_line = 0;
    state->in_error_handler = 0;

    for (int i = 0; i < 26; i++)
    {
        state->letter_types[i] = VAR_DOUBLE;
    }

    /* Data */
    state->data_ptr = 0;
    state->num_data_values = 0;
    state->data_values = NULL;
    state->capacity_data_values = 0;
    state->data_segment_line = NULL;
    state->data_segment_start = NULL;
    state->num_data_segments = 0;
    state->capacity_data_segments = 0;

    /* Files */
    state->max_files = 10;
    state->files = xcalloc(state->max_files, sizeof(FileHandle));

    /* Memory */
    state->memory_size = 32768;
    state->memory = xcalloc(state->memory_size, sizeof(unsigned char));

    /* USR */
    state->usr_address = 0;
    state->reg_a = 0;
    state->reg_b = 0;

    /* Random */
    state->random_seed = (unsigned int)time(NULL);
    state->lcg_state = state->random_seed & 0xFFFF; /* 16-bit state */
    state->last_rnd_value = 0.0;
    srand(state->random_seed);

    /* Trace */
    state->trace_on = 0;

    /* Output */
    state->output_pending = 0;
    state->output_col = 0;

    /* Other */
    state->eof_flag = 0;

    /* STOP/CONT state */
    state->stopped = 0;
    state->stop_line_number = -1;

    /* DO..LOOP stack */
    state->do_loop_cap = 64;
    state->do_loop_stack = xmalloc(state->do_loop_cap * sizeof(state->do_loop_stack[0]));
    state->do_loop_sp = 0;

    /* Scope stack for procedure local variables */
    state->scope_stack = scope_stack_create();

    /* Procedure registry for storing procedure definitions */
    state->procedure_registry = procedure_registry_create();

    /* Class registry for storing class definitions */
    state->class_registry = class_registry_create();

    /* Object instances */
    state->capacity_instances = 64;
    state->instances = xmalloc(state->capacity_instances * sizeof(ObjectInstance *));
    state->num_instances = 0;
    state->next_instance_id = 1;

    return state;
}

void runtime_free(RuntimeState *state)
{
    if (state == NULL)
    {
        return;
    }

    /* Free variables */
    if (state->variables != NULL)
    {
        for (int i = 0; i < state->num_variables; i++)
        {
            if (state->variables[i].name != NULL)
            {
                free(state->variables[i].name);
            }
            if (state->variables[i].type == VAR_STRING &&
                !state->variables[i].is_array &&
                state->variables[i].value.str_value != NULL)
            {
                free(state->variables[i].value.str_value);
            }
            if (state->variables[i].is_array && state->variables[i].dimensions != NULL)
            {
                free(state->variables[i].dimensions);
            }
            if (state->variables[i].is_array && state->variables[i].value.array_ptr != NULL)
            {
                if (state->variables[i].type == VAR_STRING)
                {
                    char **arr = (char **)state->variables[i].value.array_ptr;
                    for (int j = 0; j < state->variables[i].total_elements; j++)
                    {
                        if (arr[j])
                            free(arr[j]);
                    }
                }
                free(state->variables[i].value.array_ptr);
            }
        }
        free(state->variables);
    }

    /* Free call stack */
    if (state->call_stack != NULL)
    {
        free(state->call_stack);
    }

    /* Free data values */
    if (state->data_values != NULL)
    {
        for (int i = 0; i < state->num_data_values; i++)
        {
            if (state->data_values[i].type == VAR_STRING && state->data_values[i].str_value)
            {
                free(state->data_values[i].str_value);
            }
        }
        free(state->data_values);
    }
    free(state->data_segment_line);
    free(state->data_segment_start);

    /* Close files */
    if (state->files != NULL)
    {
        for (int i = 0; i < state->max_files; i++)
        {
            if (state->files[i].fp != NULL)
            {
                fclose(state->files[i].fp);
            }
        }
        free(state->files);
    }

    /* Free memory */
    if (state->memory != NULL)
    {
        free(state->memory);
    }

    /* Free user-defined functions */
    if (state->user_functions != NULL)
    {
        for (int i = 0; i < state->num_user_functions; i++)
        {
            if (state->user_functions[i].name != NULL)
            {
                free(state->user_functions[i].name);
            }
            if (state->user_functions[i].parameters != NULL)
            {
                for (int j = 0; j < state->user_functions[i].num_parameters; j++)
                {
                    if (state->user_functions[i].parameters[j])
                    {
                        free(state->user_functions[i].parameters[j]);
                    }
                }
                free(state->user_functions[i].parameters);
            }
            /* Note: body expression is not freed here; it's part of the program AST */
        }
        free(state->user_functions);
    }

    /* Free DO..LOOP stack */
    if (state->do_loop_stack != NULL)
    {
        free(state->do_loop_stack);
    }

    /* Free scope stack */
    if (state->scope_stack != NULL)
    {
        scope_stack_free(state->scope_stack);
    }

    /* Free procedure registry */
    if (state->procedure_registry != NULL)
    {
        procedure_registry_free(state->procedure_registry);
    }

    /* Free class registry */
    if (state->class_registry != NULL)
    {
        class_registry_free(state->class_registry);
    }

    /* Free object instances */
    if (state->instances != NULL)
    {
        for (int i = 0; i < state->num_instances; i++)
        {
            if (state->instances[i] != NULL)
            {
                runtime_free_instance(state->instances[i]);
            }
        }
        free(state->instances);
    }

    free(state);
}

void runtime_set_variable(RuntimeState *state, const char *name, double value)
{
    if (state == NULL || name == NULL)
    {
        return;
    }

    VarType type = get_var_type_from_name(state, name);
    Variable *var = ensure_variable(state, name, type);

    if (type == VAR_STRING)
    {
        /* Setting numeric value to string variable - convert */
        char buf[64];
        snprintf(buf, sizeof(buf), "%.10g", value);
        if (var->value.str_value != NULL)
        {
            free(var->value.str_value);
        }
        var->value.str_value = xstrdup(buf);
    }
    else if (type == VAR_INTEGER)
    {
        var->value.num_value = (double)((int)value);
    }
    else
    {
        var->value.num_value = value;
    }

    /* Special variables for machine code simulation */
    if (strcasecmp(name, "DEFUSR") == 0)
    {
        state->usr_address = (int)value;
    }
    else if (strcasecmp(name, "PUTA") == 0)
    {
        state->reg_a = (int)value;
    }
    else if (strcasecmp(name, "PUTB") == 0)
    {
        state->reg_b = (int)value;
    }
}

void runtime_set_string_variable(RuntimeState *state, const char *name, const char *value)
{
    if (state == NULL || name == NULL || value == NULL)
    {
        return;
    }

    VarType type = get_var_type_from_name(state, name);
    Variable *var = ensure_variable(state, name, type);
    if (var->is_array)
    {
        return;
    }

    if (type == VAR_STRING)
    {
        if (var->value.str_value != NULL)
        {
            free(var->value.str_value);
        }
        var->value.str_value = xstrdup(value);
    }
    else
    {
        /* Setting string to numeric variable - parse it */
        var->value.num_value = atof(value);
    }
}

double runtime_get_variable(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return 0.0;
    }

    VarType type = get_var_type_from_name(state, name);
    Variable *var = find_variable(state, name);

    if (var == NULL)
    {
        /* Auto-create with default value */
        var = ensure_variable(state, name, type);
    }

    if (var->type == VAR_STRING)
    {
        /* Converting string to number */
        return var->value.str_value ? atof(var->value.str_value) : 0.0;
    }

    return var->value.num_value;
}

int runtime_has_variable(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return 0;
    }

    Variable *var = find_variable(state, name);
    return var != NULL ? 1 : 0;
}

void runtime_delete_variable(RuntimeState *state, const char *name)
{
    /* Note: Variable deletion from symtable not fully implemented */
    /* For now, we'll just set it to 0/empty string */
    if (state == NULL || name == NULL)
    {
        return;
    }

    VarType type = get_var_type_from_name(state, name);
    if (type == VAR_STRING)
    {
        runtime_set_string_variable(state, name, "");
    }
    else
    {
        runtime_set_variable(state, name, 0.0);
    }
}

char *runtime_get_string_variable(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return xstrdup("");
    }

    VarType type = get_var_type_from_name(state, name);
    Variable *var = find_variable(state, name);

    if (var == NULL)
    {
        /* Auto-create with default value */
        var = ensure_variable(state, name, type);
    }

    if (var->type == VAR_STRING)
    {
        return xstrdup(var->value.str_value ? var->value.str_value : "");
    }
    else
    {
        /* Convert number to string */
        char buf[64];
        if (fabs(var->value.num_value) < 1e-10 && var->value.num_value != 0.0)
            snprintf(buf, sizeof(buf), "%.9e", var->value.num_value);
        else
            snprintf(buf, sizeof(buf), "%.15g", var->value.num_value);
        return xstrdup(buf);
    }
}

void runtime_dim_array(RuntimeState *state, const char *name, int *dimensions, int num_dims)
{
    if (state == NULL || name == NULL || dimensions == NULL || num_dims <= 0)
    {
        return;
    }

    VarType type = get_var_type_from_name(state, name);
    Variable *var = ensure_variable(state, name, type);

    /* Calculate total elements */
    int total = 1;
    for (int i = 0; i < num_dims; i++)
    {
        total *= (dimensions[i] + 1); /* BASIC arrays are 0-indexed with upper bound */
    }

    /* Free old array if exists */
    if (var->is_array && var->value.array_ptr != NULL)
    {
        if (var->type == VAR_STRING)
        {
            char **arr = (char **)var->value.array_ptr;
            for (int i = 0; i < var->total_elements; i++)
            {
                if (arr[i])
                    free(arr[i]);
            }
        }
        free(var->value.array_ptr);
    }
    if (var->dimensions != NULL)
    {
        free(var->dimensions);
    }

    /* Allocate new array */
    var->is_array = 1;
    var->num_dimensions = num_dims;
    var->dimensions = xmalloc(num_dims * sizeof(int));
    memcpy(var->dimensions, dimensions, num_dims * sizeof(int));
    var->total_elements = total;

    if (type == VAR_STRING)
    {
        var->value.array_ptr = xcalloc(total, sizeof(char *));
    }
    else
    {
        /* Numeric array - allocate as doubles */
        var->value.array_ptr = (int *)xcalloc(total, sizeof(double));
    }
}

void runtime_set_array_element(RuntimeState *state, const char *name, int *indices, int num_indices, double value)
{
    if (state == NULL || name == NULL || indices == NULL)
    {
        return;
    }

    Variable *var = find_variable(state, name);
    if (var == NULL || !var->is_array)
    {
        return; /* Array not defined */
    }

    if (num_indices != var->num_dimensions)
    {
        return;
    }

    /* Calculate linear index */
    int index = 0;
    int multiplier = 1;
    for (int i = num_indices - 1; i >= 0; i--)
    {
        index += indices[i] * multiplier;
        multiplier *= (var->dimensions[i] + 1);
    }

    if (index >= 0 && index < var->total_elements)
    {
        double *array = (double *)var->value.array_ptr;
        array[index] = value;
    }
}

double runtime_get_array_element(RuntimeState *state, const char *name, int *indices, int num_indices)
{
    if (state == NULL || name == NULL || indices == NULL)
    {
        return 0.0;
    }

    Variable *var = find_variable(state, name);
    if (var == NULL || !var->is_array)
    {
        return 0.0; /* Array not defined */
    }

    if (num_indices != var->num_dimensions)
    {
        return 0.0;
    }

    /* Calculate linear index */
    int index = 0;
    int multiplier = 1;
    for (int i = num_indices - 1; i >= 0; i--)
    {
        index += indices[i] * multiplier;
        multiplier *= (var->dimensions[i] + 1);
    }

    if (index >= 0 && index < var->total_elements)
    {
        double *array = (double *)var->value.array_ptr;
        return array[index];
    }

    return 0.0;
}

void runtime_set_string_array_element(RuntimeState *state, const char *name, int *indices, int num_indices, const char *value)
{
    if (state == NULL || name == NULL || indices == NULL)
    {
        return;
    }

    Variable *var = find_variable(state, name);
    if (var == NULL || !var->is_array || var->type != VAR_STRING)
    {
        return;
    }

    if (num_indices != var->num_dimensions)
    {
        return;
    }

    int index = 0;
    int multiplier = 1;
    for (int i = var->num_dimensions - 1; i >= 0; i--)
    {
        if (indices[i] < 0 || indices[i] > var->dimensions[i])
        {
            return;
        }
        index += indices[i] * multiplier;
        multiplier *= (var->dimensions[i] + 1);
    }

    char **arr = (char **)var->value.array_ptr;
    if (arr[index])
    {
        free(arr[index]);
    }
    arr[index] = xstrdup(value ? value : "");
}

char *runtime_get_string_array_element(RuntimeState *state, const char *name, int *indices, int num_indices)
{
    if (state == NULL || name == NULL || indices == NULL)
    {
        return xstrdup("");
    }

    Variable *var = find_variable(state, name);
    if (var == NULL || !var->is_array || var->type != VAR_STRING)
    {
        return xstrdup("");
    }

    if (num_indices != var->num_dimensions)
    {
        return xstrdup("");
    }

    int index = 0;
    int multiplier = 1;
    for (int i = var->num_dimensions - 1; i >= 0; i--)
    {
        if (indices[i] < 0 || indices[i] > var->dimensions[i])
        {
            return xstrdup("");
        }
        index += indices[i] * multiplier;
        multiplier *= (var->dimensions[i] + 1);
    }

    char **arr = (char **)var->value.array_ptr;
    return xstrdup(arr[index] ? arr[index] : "");
}

int runtime_push_call(RuntimeState *state, int return_line)
{
    if (state == NULL || state->call_stack_ptr >= state->call_stack_capacity)
    {
        return 0;
    }
    state->call_stack[state->call_stack_ptr++] = return_line;
    return 1;
}

int runtime_pop_call(RuntimeState *state)
{
    if (state == NULL || state->call_stack_ptr == 0)
    {
        return -1;
    }
    return state->call_stack[--state->call_stack_ptr];
}

void runtime_set_error(RuntimeState *state, int code, int line)
{
    if (state != NULL)
    {
        state->error_code = code;
        state->error_line = line;
    }
}

int runtime_get_error(RuntimeState *state)
{
    return state ? state->error_code : 0;
}

int runtime_get_error_line(RuntimeState *state)
{
    return state ? state->error_line : 0;
}

void runtime_clear_error(RuntimeState *state)
{
    if (state != NULL)
    {
        state->error_code = 0;
        state->error_line = 0;
    }
}

void runtime_clear_all(RuntimeState *state)
{
    if (state == NULL)
        return;

    /* Clear all variables and arrays */
    for (int i = 0; i < state->num_variables; i++)
    {
        free(state->variables[i].name);
        if (state->variables[i].type == VAR_STRING && state->variables[i].value.str_value)
        {
            free(state->variables[i].value.str_value);
        }
        if (state->variables[i].is_array)
        {
            free(state->variables[i].dimensions);
            if (state->variables[i].type == VAR_STRING && state->variables[i].value.array_ptr)
            {
                /* Arrays of strings store pointers - would need proper cleanup */
                /* For now, just free the array structure */
            }
        }
    }

    /* Reset the variable array */
    state->num_variables = 0;
}

void runtime_set_error_handler(RuntimeState *state, int line)
{
    if (state != NULL)
    {
        state->error_handler_line = line;
    }
}

int runtime_get_error_handler(RuntimeState *state)
{
    return state ? state->error_handler_line : 0;
}

void runtime_set_in_error_handler(RuntimeState *state, int in_handler)
{
    if (state != NULL)
    {
        state->in_error_handler = in_handler ? 1 : 0;
    }
}

int runtime_is_in_error_handler(RuntimeState *state)
{
    return state ? state->in_error_handler : 0;
}

double runtime_random(RuntimeState *state)
{
    if (state == NULL)
    {
        return 0.0;
    }

    /* LCG formula: X_{n+1} = (a * X_n + c) mod m
     * Parameters for TRS-80 Level II BASIC:
     * a = 75, c = 74, m = 65536 (2^16)
     */
    unsigned int a = 75;
    unsigned int c = 74;

    state->lcg_state = (a * state->lcg_state + c) & 0xFFFF; /* mod 65536 using mask */
    state->last_rnd_value = (double)state->lcg_state / 65536.0;

    return state->last_rnd_value;
}

void runtime_randomize(RuntimeState *state, int seed)
{
    if (state != NULL)
    {
        state->random_seed = (unsigned int)seed;
        state->lcg_state = state->random_seed & 0xFFFF; /* 16-bit state */
        state->last_rnd_value = 0.0;
    }
}

double runtime_get_last_rnd(RuntimeState *state)
{
    return state ? state->last_rnd_value : 0.0;
}

void runtime_data_reset(RuntimeState *state)
{
    if (state != NULL)
    {
        state->data_ptr = 0;
    }
}

void runtime_data_clear(RuntimeState *state)
{
    if (state == NULL)
    {
        return;
    }
    if (state->data_values != NULL)
    {
        for (int i = 0; i < state->num_data_values; i++)
        {
            if (state->data_values[i].type == VAR_STRING && state->data_values[i].str_value)
            {
                free(state->data_values[i].str_value);
            }
        }
        free(state->data_values);
    }
    state->data_values = NULL;
    state->num_data_values = 0;
    state->capacity_data_values = 0;
    state->data_ptr = 0;
    free(state->data_segment_line);
    free(state->data_segment_start);
    state->data_segment_line = NULL;
    state->data_segment_start = NULL;
    state->num_data_segments = 0;
    state->capacity_data_segments = 0;
}

static void ensure_data_segment_capacity(RuntimeState *state)
{
    if (state->num_data_segments >= state->capacity_data_segments)
    {
        state->capacity_data_segments = (state->capacity_data_segments == 0) ? 16 : state->capacity_data_segments * 2;
        state->data_segment_line = xrealloc(state->data_segment_line,
                                            (size_t)state->capacity_data_segments * sizeof(int));
        state->data_segment_start = xrealloc(state->data_segment_start,
                                             (size_t)state->capacity_data_segments * sizeof(int));
    }
}

void runtime_data_start_segment(RuntimeState *state, int line_number)
{
    if (state == NULL)
        return;
    ensure_data_segment_capacity(state);
    state->data_segment_line[state->num_data_segments] = line_number;
    state->data_segment_start[state->num_data_segments] = state->num_data_values;
    state->num_data_segments++;
}

void runtime_data_reset_to_line(RuntimeState *state, int line_number)
{
    if (state == NULL)
    {
        return;
    }
    /* Find first DATA segment at or after line_number */
    for (int i = 0; i < state->num_data_segments; i++)
    {
        if (state->data_segment_line[i] >= line_number)
        {
            state->data_ptr = state->data_segment_start[i];
            return;
        }
    }
    /* No segment at or after line; reset to start */
    state->data_ptr = 0;
}

static void ensure_data_capacity(RuntimeState *state)
{
    if (state->num_data_values >= state->capacity_data_values)
    {
        state->capacity_data_values = (state->capacity_data_values == 0) ? 64 : state->capacity_data_values * 2;
        state->data_values = xrealloc(state->data_values, state->capacity_data_values * sizeof(DataValue));
    }
}

void runtime_data_add_number(RuntimeState *state, double value)
{
    if (state == NULL)
    {
        return;
    }
    ensure_data_capacity(state);
    DataValue *dv = &state->data_values[state->num_data_values++];
    dv->type = VAR_DOUBLE;
    dv->num_value = value;
    dv->str_value = NULL;
}

void runtime_data_add_string(RuntimeState *state, const char *value)
{
    if (state == NULL)
    {
        return;
    }
    ensure_data_capacity(state);
    DataValue *dv = &state->data_values[state->num_data_values++];
    dv->type = VAR_STRING;
    dv->num_value = 0.0;
    dv->str_value = xstrdup(value ? value : "");
}

int runtime_data_read(RuntimeState *state, VarType *out_type, double *out_num, char **out_str)
{
    if (state == NULL || state->data_ptr >= state->num_data_values)
    {
        return 0;
    }

    DataValue *dv = &state->data_values[state->data_ptr++];
    if (out_type)
    {
        *out_type = dv->type;
    }
    if (dv->type == VAR_STRING)
    {
        if (out_str)
        {
            *out_str = xstrdup(dv->str_value ? dv->str_value : "");
        }
        if (out_num)
        {
            *out_num = 0.0;
        }
    }
    else
    {
        if (out_num)
        {
            *out_num = dv->num_value;
        }
        if (out_str)
        {
            *out_str = NULL;
        }
    }
    return 1;
}

int runtime_open_file(RuntimeState *state, int handle, const char *filename, const char *mode)
{
    if (state == NULL || handle <= 0 || handle > state->max_files || filename == NULL || mode == NULL)
    {
        return 0;
    }
    int idx = handle - 1;
    if (state->files[idx].fp != NULL)
    {
        fclose(state->files[idx].fp);
    }
    state->files[idx].fp = fopen(filename, mode);
    state->files[idx].mode = (mode[0] == 'r') ? 1 : (mode[0] == 'a' ? 3 : 2);
    return state->files[idx].fp != NULL;
}

void runtime_close_file(RuntimeState *state, int handle)
{
    if (state == NULL || handle <= 0 || handle > state->max_files)
    {
        return;
    }
    int idx = handle - 1;
    if (state->files[idx].fp != NULL)
    {
        fclose(state->files[idx].fp);
        state->files[idx].fp = NULL;
        state->files[idx].mode = 0;
    }
}

FILE *runtime_get_file(RuntimeState *state, int handle)
{
    if (state == NULL || handle <= 0 || handle > state->max_files)
    {
        return NULL;
    }
    return state->files[handle - 1].fp;
}

int runtime_file_eof(RuntimeState *state, int handle)
{
    FILE *fp = runtime_get_file(state, handle);
    if (!fp)
    {
        return 1;
    }
    if (feof(fp))
    {
        return 1;
    }
    int ch = fgetc(fp);
    if (ch == EOF)
    {
        return 1;
    }
    ungetc(ch, fp);
    return 0;
}

long runtime_file_loc(RuntimeState *state, int handle)
{
    FILE *fp = runtime_get_file(state, handle);
    if (!fp)
    {
        return 0;
    }
    long pos = ftell(fp);
    return (pos < 0) ? 0 : pos;
}

long runtime_file_lof(RuntimeState *state, int handle)
{
    FILE *fp = runtime_get_file(state, handle);
    if (!fp)
    {
        return 0;
    }
    long cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long end = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return (end < 0) ? 0 : end;
}

int runtime_file_get(RuntimeState *state, int handle, int *out_byte)
{
    FILE *fp = runtime_get_file(state, handle);
    if (!fp)
    {
        return 0;
    }
    int ch = fgetc(fp);
    if (ch == EOF)
    {
        return 0;
    }
    if (out_byte)
    {
        *out_byte = ch & 0xFF;
    }
    return 1;
}

int runtime_file_put(RuntimeState *state, int handle, int byte_val)
{
    FILE *fp = runtime_get_file(state, handle);
    if (!fp)
    {
        return 0;
    }
    fputc(byte_val & 0xFF, fp);
    fflush(fp);
    return 1;
}

void runtime_poke(RuntimeState *state, int addr, int value)
{
    /* Handle regular memory */
    if (state == NULL || addr < 0 || addr >= state->memory_size)
    {
        return;
    }
    state->memory[addr] = (unsigned char)(value & 0xFF);
}

int runtime_peek(RuntimeState *state, int addr)
{
    /* Handle regular memory */
    if (state == NULL || addr < 0 || addr >= state->memory_size)
    {
        return 0;
    }
    return (int)state->memory[addr];
}

void runtime_set_usr_address(RuntimeState *state, int addr)
{
    if (state != NULL)
    {
        state->usr_address = addr;
    }
}

int runtime_get_usr_address(RuntimeState *state)
{
    return state ? state->usr_address : 0;
}

void runtime_set_reg_a(RuntimeState *state, int value)
{
    if (state != NULL)
    {
        state->reg_a = value;
    }
}

void runtime_set_reg_b(RuntimeState *state, int value)
{
    if (state != NULL)
    {
        state->reg_b = value;
    }
}

int runtime_get_reg_a(RuntimeState *state)
{
    return state ? state->reg_a : 0;
}

int runtime_get_reg_b(RuntimeState *state)
{
    return state ? state->reg_b : 0;
}

int runtime_get_var_address(RuntimeState *state, const char *name)
{
    Variable *var = find_variable(state, name);
    return var ? var->address : 0;
}

VarType runtime_get_variable_type(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return VAR_DOUBLE;
    }

    /* First check if variable exists */
    Variable *var = find_variable(state, name);
    if (var != NULL)
    {
        return var->type;
    }

    /* Infer from name suffix */
    return get_var_type_from_name(state, name);
}

void runtime_set_trace(RuntimeState *state, int trace_on)
{
    if (state)
    {
        state->trace_on = trace_on;
    }
}

int runtime_get_trace(RuntimeState *state)
{
    return state ? state->trace_on : 0;
}

void runtime_set_memory_size(RuntimeState *state, int memory_size)
{
    if (state == NULL)
    {
        return;
    }

    if (memory_size <= 0)
    {
        memory_size = 32768;
    }

    if (memory_size > 65536)
    {
        memory_size = 65536;
    }

    if (state->memory != NULL)
    {
        free(state->memory);
    }

    state->memory_size = memory_size;
    state->memory = xcalloc(state->memory_size, sizeof(unsigned char));
}

int runtime_get_memory_size(RuntimeState *state)
{
    return state ? state->memory_size : 0;
}

void runtime_set_output_pending(RuntimeState *state, int pending)
{
    if (state)
    {
        state->output_pending = pending;
    }
}

int runtime_get_output_pending(RuntimeState *state)
{
    return state ? state->output_pending : 0;
}

void runtime_set_output_col(RuntimeState *state, int col)
{
    if (state)
    {
        state->output_col = col;
    }
}

int runtime_get_output_col(RuntimeState *state)
{
    return state ? state->output_col : 0;
}

void runtime_set_save_callback(RuntimeState *state, SaveCallback callback)
{
    if (state)
    {
        state->save_callback = callback;
    }
}

SaveCallback runtime_get_save_callback(RuntimeState *state)
{
    return state ? state->save_callback : NULL;
}
void runtime_set_delete_callback(RuntimeState *state, DeleteCallback callback)
{
    if (state)
    {
        state->delete_callback = callback;
    }
}

DeleteCallback runtime_get_delete_callback(RuntimeState *state)
{
    return state ? state->delete_callback : NULL;
}

void runtime_set_merge_callback(RuntimeState *state, MergeCallback callback)
{
    if (state)
    {
        state->merge_callback = callback;
    }
}

MergeCallback runtime_get_merge_callback(RuntimeState *state)
{
    return state ? state->merge_callback : NULL;
}

int runtime_get_last_entered_line(RuntimeState *state)
{
    return state ? state->last_entered_line : 0;
}

void runtime_set_last_entered_line(RuntimeState *state, int line_number)
{
    if (state)
    {
        state->last_entered_line = line_number;
    }
}

/* User-defined function support */

int runtime_define_function(RuntimeState *state, const char *name,
                            const char **parameters, int num_parameters,
                            struct ASTExpr *body)
{
    if (state == NULL || name == NULL)
    {
        return 0;
    }

    /* Check if function already exists and replace it */
    for (int i = 0; i < state->num_user_functions; i++)
    {
        if (strcmp(state->user_functions[i].name, name) == 0)
        {
            /* Free old parameters */
            if (state->user_functions[i].parameters != NULL)
            {
                for (int j = 0; j < state->user_functions[i].num_parameters; j++)
                {
                    if (state->user_functions[i].parameters[j])
                    {
                        free(state->user_functions[i].parameters[j]);
                    }
                }
                free(state->user_functions[i].parameters);
            }
            /* Replace function */
            state->user_functions[i].body = body;
            state->user_functions[i].num_parameters = num_parameters;
            if (num_parameters > 0 && parameters != NULL)
            {
                state->user_functions[i].parameters = xmalloc(num_parameters * sizeof(char *));
                for (int j = 0; j < num_parameters; j++)
                {
                    state->user_functions[i].parameters[j] = xstrdup(parameters[j]);
                }
            }
            else
            {
                state->user_functions[i].parameters = NULL;
            }
            return 1;
        }
    }

    /* Expand capacity if needed */
    if (state->num_user_functions >= state->capacity_user_functions)
    {
        state->capacity_user_functions *= 2;
        state->user_functions = xrealloc(state->user_functions,
                                         state->capacity_user_functions * sizeof(UserDefinedFunction));
    }

    /* Add new function */
    UserDefinedFunction *fn = &state->user_functions[state->num_user_functions++];
    fn->name = xstrdup(name);
    fn->body = body;
    fn->num_parameters = num_parameters;

    if (num_parameters > 0 && parameters != NULL)
    {
        fn->parameters = xmalloc(num_parameters * sizeof(char *));
        for (int i = 0; i < num_parameters; i++)
        {
            fn->parameters[i] = xstrdup(parameters[i]);
        }
    }
    else
    {
        fn->parameters = NULL;
    }

    return 1;
}

struct ASTExpr *runtime_get_function_body(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < state->num_user_functions; i++)
    {
        if (strcmp(state->user_functions[i].name, name) == 0)
        {
            return state->user_functions[i].body;
        }
    }
    return NULL;
}

const char **runtime_get_function_params(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < state->num_user_functions; i++)
    {
        if (strcmp(state->user_functions[i].name, name) == 0)
        {
            return (const char **)state->user_functions[i].parameters;
        }
    }
    return NULL;
}

int runtime_get_function_param_count(RuntimeState *state, const char *name)
{
    if (state == NULL || name == NULL)
    {
        return 0;
    }

    for (int i = 0; i < state->num_user_functions; i++)
    {
        if (strcmp(state->user_functions[i].name, name) == 0)
        {
            return state->user_functions[i].num_parameters;
        }
    }
    return 0;
}

/* STOP/CONT support */
void runtime_set_stop_state(RuntimeState *state, int line_number)
{
    if (state == NULL)
    {
        return;
    }
    state->stopped = 1;
    state->stop_line_number = line_number;
}

void runtime_clear_stop_state(RuntimeState *state)
{
    if (state == NULL)
    {
        return;
    }
    state->stopped = 0;
    state->stop_line_number = -1;
}

int runtime_is_stopped(RuntimeState *state)
{
    if (state == NULL)
    {
        return 0;
    }
    return state->stopped;
}

int runtime_get_stop_line(RuntimeState *state)
{
    if (state == NULL)
    {
        return -1;
    }
    return state->stop_line_number;
}

/* DO..LOOP stack management */
int runtime_push_do_loop(RuntimeState *state, int do_line, int condition_type, void *condition)
{
    if (state == NULL || state->do_loop_stack == NULL)
    {
        return 0;
    }

    if (state->do_loop_sp >= state->do_loop_cap)
    {
        /* Expand capacity */
        state->do_loop_cap *= 2;
        state->do_loop_stack = xrealloc(state->do_loop_stack,
                                        state->do_loop_cap * sizeof(state->do_loop_stack[0]));
    }

    int sp = state->do_loop_sp++;
    state->do_loop_stack[sp].do_line_index = do_line;
    state->do_loop_stack[sp].loop_line_index = -1;
    state->do_loop_stack[sp].condition_type = condition_type;
    state->do_loop_stack[sp].condition = condition;

    return 1;
}

int runtime_pop_do_loop(RuntimeState *state, int *out_loop_line)
{
    if (state == NULL || state->do_loop_sp <= 0)
    {
        return 0;
    }

    int sp = state->do_loop_sp - 1;
    if (out_loop_line)
    {
        *out_loop_line = state->do_loop_stack[sp].loop_line_index;
    }
    state->do_loop_sp--;

    return 1;
}

int runtime_get_do_loop_depth(RuntimeState *state)
{
    if (state == NULL)
    {
        return 0;
    }
    return state->do_loop_sp;
}

int runtime_get_current_do_line(RuntimeState *state)
{
    if (state == NULL || state->do_loop_sp <= 0)
    {
        return -1;
    }
    int sp = state->do_loop_sp - 1;
    return state->do_loop_stack[sp].do_line_index;
}

int runtime_set_current_loop_line(RuntimeState *state, int loop_line)
{
    if (state == NULL || state->do_loop_sp <= 0)
    {
        return 0;
    }
    int sp = state->do_loop_sp - 1;
    state->do_loop_stack[sp].loop_line_index = loop_line;
    return 1;
}

int runtime_get_current_condition_type(RuntimeState *state)
{
    if (state == NULL || state->do_loop_sp <= 0)
    {
        return 0;
    }
    int sp = state->do_loop_sp - 1;
    return state->do_loop_stack[sp].condition_type;
}

void *runtime_get_current_condition(RuntimeState *state)
{
    if (state == NULL || state->do_loop_sp <= 0)
    {
        return NULL;
    }
    int sp = state->do_loop_sp - 1;
    return state->do_loop_stack[sp].condition;
}
/* Variable iteration for debugging/tracing */
int runtime_get_var_count(RuntimeState *state)
{
    if (state == NULL)
        return 0;
    return state->num_variables;
}

int runtime_get_var_by_index(RuntimeState *state, int index, RuntimeVar *out_var)
{
    if (state == NULL || out_var == NULL || index < 0 || index >= state->num_variables)
        return -1;

    Variable *var = &state->variables[index];
    out_var->name = var->name;
    out_var->is_string = (var->type == VAR_STRING);
    out_var->is_array = var->is_array;
    out_var->numeric_value = var->value.num_value;
    out_var->string_value = var->value.str_value;
    return 0;
}

/* ============================================================================
 * SCOPE STACK IMPLEMENTATION - For procedure local variables
 * ============================================================================ */

ScopeStack *scope_stack_create(void)
{
    ScopeStack *stack = xcalloc(1, sizeof(ScopeStack));
    stack->capacity = 32;
    stack->stack = xmalloc(stack->capacity * sizeof(Scope *));
    stack->depth = 0;
    stack->next_scope_id = 1;
    return stack;
}

void scope_stack_free(ScopeStack *stack)
{
    if (stack == NULL)
        return;

    /* Free all scopes on the stack */
    if (stack->stack != NULL)
    {
        for (int i = 0; i < stack->depth; i++)
        {
            if (stack->stack[i] != NULL)
            {
                scope_free(stack->stack[i]);
            }
        }
        free(stack->stack);
    }

    free(stack);
}

Scope *scope_create(Scope *parent)
{
    Scope *scope = xcalloc(1, sizeof(Scope));
    scope->scope_id = 0;      /* Will be set by scope_stack */
    scope->local_vars = NULL; /* Will be initialized by caller if needed */
    scope->parent = parent;
    return scope;
}

void scope_free(Scope *scope)
{
    if (scope == NULL)
        return;

    /* Note: local_vars (SymbolTable*) should be freed by caller
     * because we use void* to avoid circular dependency */
    free(scope);
}

void scope_push(ScopeStack *stack, Scope *scope)
{
    if (stack == NULL || scope == NULL)
        return;

    /* Resize if needed */
    if (stack->depth >= stack->capacity)
    {
        stack->capacity *= 2;
        stack->stack = xrealloc(stack->stack, stack->capacity * sizeof(Scope *));
    }

    /* Assign scope ID and set parent to current scope */
    scope->scope_id = stack->next_scope_id++;
    if (stack->depth > 0)
    {
        scope->parent = stack->stack[stack->depth - 1];
    }

    stack->stack[stack->depth++] = scope;
}

Scope *scope_pop(ScopeStack *stack)
{
    if (stack == NULL || stack->depth == 0)
        return NULL;

    return stack->stack[--stack->depth];
}

Scope *scope_current(ScopeStack *stack)
{
    if (stack == NULL || stack->depth == 0)
        return NULL;

    return stack->stack[stack->depth - 1];
}

Scope *scope_lookup_chain(Scope *scope, const char *var_name)
{
    /* Walk up the scope chain looking for the variable name
     * Returns the scope where the variable is defined, or NULL */

    if (scope == NULL || var_name == NULL)
        return NULL;

    /* For now, just return current scope
     * Full implementation would check local_vars SymbolTable
     * This is a placeholder for future integration */
    return scope;
}

/* Procedure Registry Implementation */

ProcedureRegistry *procedure_registry_create(void)
{
    ProcedureRegistry *reg = xcalloc(1, sizeof(ProcedureRegistry));
    reg->capacity = 32;
    reg->procedures = xmalloc(reg->capacity * sizeof(ProcedureDef *));
    reg->count = 0;
    return reg;
}

void procedure_registry_free(ProcedureRegistry *reg)
{
    if (reg == NULL)
        return;

    /* Free each procedure definition */
    for (int i = 0; i < reg->count; i++)
    {
        if (reg->procedures[i] != NULL)
        {
            if (reg->procedures[i]->name != NULL)
                free(reg->procedures[i]->name);
            /* parameters and body are managed by AST, don't free here */
            free(reg->procedures[i]);
        }
    }

    free(reg->procedures);
    free(reg);
}

void procedure_registry_add(ProcedureRegistry *reg, const char *name, void *parameters, void *body)
{
    if (reg == NULL || name == NULL)
        return;

    /* Resize if needed */
    if (reg->count >= reg->capacity)
    {
        reg->capacity *= 2;
        reg->procedures = xrealloc(reg->procedures, reg->capacity * sizeof(ProcedureDef *));
    }

    /* Create new procedure definition */
    ProcedureDef *proc = xmalloc(sizeof(ProcedureDef));
    proc->name = xstrdup(name);
    proc->parameters = parameters;
    proc->body = body;

    reg->procedures[reg->count] = proc;
    reg->count++;
}

ProcedureDef *procedure_registry_lookup(ProcedureRegistry *reg, const char *name)
{
    if (reg == NULL || name == NULL)
        return NULL;

    for (int i = 0; i < reg->count; i++)
    {
        if (strcmp(reg->procedures[i]->name, name) == 0)
            return reg->procedures[i];
    }

    return NULL;
}

void procedure_registry_clear(ProcedureRegistry *reg)
{
    if (reg == NULL)
        return;

    for (int i = 0; i < reg->count; i++)
    {
        if (reg->procedures[i] != NULL)
        {
            if (reg->procedures[i]->name != NULL)
                free(reg->procedures[i]->name);
            free(reg->procedures[i]);
        }
    }

    reg->count = 0;
}

/* RuntimeState procedure access functions */

void runtime_register_procedure(RuntimeState *state, const char *name, void *parameters, void *body)
{
    if (state == NULL || state->procedure_registry == NULL)
        return;

    procedure_registry_add(state->procedure_registry, name, parameters, body);
}

ProcedureDef *runtime_lookup_procedure(RuntimeState *state, const char *name)
{
    if (state == NULL || state->procedure_registry == NULL)
        return NULL;

    return procedure_registry_lookup(state->procedure_registry, name);
}

ProcedureRegistry *runtime_get_procedure_registry(RuntimeState *state)
{
    if (state == NULL)
        return NULL;

    return state->procedure_registry;
}

/* Scope stack access through RuntimeState */

ScopeStack *runtime_get_scope_stack(RuntimeState *state)
{
    if (state == NULL)
        return NULL;

    return state->scope_stack;
}
/* Class Registry Implementation */

ClassRegistry *class_registry_create(void)
{
    ClassRegistry *reg = xcalloc(1, sizeof(ClassRegistry));
    reg->capacity = 32;
    reg->classes = xmalloc(reg->capacity * sizeof(ClassDef *));
    reg->count = 0;
    return reg;
}

void class_registry_free(ClassRegistry *reg)
{
    if (reg == NULL)
        return;

    /* Free each class definition */
    for (int i = 0; i < reg->count; i++)
    {
        if (reg->classes[i] != NULL)
        {
            if (reg->classes[i]->name != NULL)
                free(reg->classes[i]->name);
            /* parameters and body are managed by AST, don't free here */
            free(reg->classes[i]);
        }
    }

    free(reg->classes);
    free(reg);
}

void class_registry_add(ClassRegistry *reg, const char *name, void *parameters, void *body)
{
    if (reg == NULL || name == NULL)
        return;

    /* Resize if needed */
    if (reg->count >= reg->capacity)
    {
        reg->capacity *= 2;
        reg->classes = xrealloc(reg->classes, reg->capacity * sizeof(ClassDef *));
    }

    /* Create new class definition */
    ClassDef *cls = xmalloc(sizeof(ClassDef));
    cls->name = xstrdup(name);
    cls->parameters = parameters;  /* Member variables */
    cls->body = body;              /* Method definitions */
    cls->method_procedures = NULL; /* Will be populated during parsing */

    reg->classes[reg->count] = cls;
    reg->count++;
}

ClassDef *class_registry_lookup(ClassRegistry *reg, const char *name)
{
    if (reg == NULL || name == NULL)
        return NULL;

    for (int i = 0; i < reg->count; i++)
    {
        if (strcmp(reg->classes[i]->name, name) == 0)
            return reg->classes[i];
    }

    return NULL;
}

/* RuntimeState class access functions */

void runtime_register_class(RuntimeState *state, const char *name, void *parameters, void *body)
{
    if (state == NULL || state->class_registry == NULL)
        return;

    class_registry_add(state->class_registry, name, parameters, body);
}

ClassDef *runtime_lookup_class(RuntimeState *state, const char *name)
{
    if (state == NULL || state->class_registry == NULL)
        return NULL;

    return class_registry_lookup(state->class_registry, name);
}

ClassRegistry *runtime_get_class_registry(RuntimeState *state)
{
    if (state == NULL)
        return NULL;

    return state->class_registry;
}

/* Object Instance Management */

ObjectInstance *runtime_create_instance(RuntimeState *state, const char *class_name)
{
    if (state == NULL || class_name == NULL)
        return NULL;

    /* Look up the class definition */
    ClassDef *class_def = runtime_lookup_class(state, class_name);
    if (class_def == NULL)
        return NULL;

    /* Create a new instance */
    ObjectInstance *instance = xcalloc(1, sizeof(ObjectInstance));
    instance->class_name = xstrdup(class_name);
    instance->instance_id = state->next_instance_id++;

    /* Create a scope for instance variables */
    instance->instance_scope = scope_create(scope_current(state->scope_stack));

    /* Add instance to runtime's instance list */
    if (state->num_instances >= state->capacity_instances)
    {
        state->capacity_instances *= 2;
        state->instances = xrealloc(state->instances, state->capacity_instances * sizeof(ObjectInstance *));
    }

    state->instances[state->num_instances] = instance;
    state->num_instances++;

    return instance;
}

void runtime_free_instance(ObjectInstance *instance)
{
    if (instance == NULL)
        return;

    if (instance->class_name != NULL)
        free(instance->class_name);

    if (instance->instance_scope != NULL)
        scope_free((Scope *)instance->instance_scope);

    free(instance);
}

ObjectInstance *runtime_get_instance(RuntimeState *state, int instance_id)
{
    if (state == NULL)
        return NULL;

    for (int i = 0; i < state->num_instances; i++)
    {
        if (state->instances[i] != NULL && state->instances[i]->instance_id == instance_id)
            return state->instances[i];
    }

    return NULL;
}

/* Instance variable access */

void runtime_set_instance_variable(ObjectInstance *instance, const char *var_name, double value)
{
    if (instance == NULL || var_name == NULL)
        return;

    /* For now, store instance variables in RuntimeState with instance-prefixed names
     * Format: __INST<id>_<var_name> */
    RuntimeState *state = runtime_get_current_state();
    if (state)
    {
        char full_name[256];
        snprintf(full_name, sizeof(full_name), "__INST%d_%s", instance->instance_id, var_name);
        runtime_set_variable(state, full_name, value);
    }
}

void runtime_set_instance_string_variable(ObjectInstance *instance, const char *var_name, const char *value)
{
    if (instance == NULL || var_name == NULL)
        return;

    /* Integration with instance scope's symbol table */
}

double runtime_get_instance_variable(ObjectInstance *instance, const char *var_name)
{
    if (instance == NULL || var_name == NULL)
        return 0.0;

    /* Get instance variables from RuntimeState with instance-prefixed names */
    RuntimeState *state = runtime_get_current_state();
    if (state)
    {
        char full_name[256];
        snprintf(full_name, sizeof(full_name), "__INST%d_%s", instance->instance_id, var_name);
        return runtime_get_variable(state, full_name);
    }

    return 0.0;
}

char *runtime_get_instance_string_variable(ObjectInstance *instance, const char *var_name)
{
    if (instance == NULL || var_name == NULL)
        return NULL;

    /* Integration with instance scope's symbol table */
    return NULL;
}