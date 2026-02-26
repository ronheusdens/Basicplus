#ifndef RUNTIME_H
#define RUNTIME_H

#include "common.h"
#include <stdio.h>

/*
 * Runtime value
 */
typedef union
{
    double num_value;
    char *str_value;
    int *array_ptr;
} RuntimeValue;

/*
 * Runtime state (opaque structure, defined in runtime.c)
 */
typedef struct RuntimeState RuntimeState;

/* Runtime functions */

RuntimeState *runtime_create(void);
void runtime_free(RuntimeState *state);

/* Current execution state (set by executor for ast_eval_expr) */
void runtime_set_current_state(RuntimeState *state);
RuntimeState *runtime_get_current_state(void);

void runtime_set_memory_size(RuntimeState *state, int memory_size);
int runtime_get_memory_size(RuntimeState *state);

void runtime_set_output_pending(RuntimeState *state, int pending);
int runtime_get_output_pending(RuntimeState *state);
void runtime_set_output_col(RuntimeState *state, int col);
int runtime_get_output_col(RuntimeState *state);

void runtime_set_variable(RuntimeState *state, const char *name, double value);
double runtime_get_variable(RuntimeState *state, const char *name);
void runtime_set_string_variable(RuntimeState *state, const char *name, const char *value);
char *runtime_get_string_variable(RuntimeState *state, const char *name);

void runtime_dim_array(RuntimeState *state, const char *name, int *dimensions, int num_dims);
void runtime_set_array_element(RuntimeState *state, const char *name, int *indices, int num_indices, double value);
double runtime_get_array_element(RuntimeState *state, const char *name, int *indices, int num_indices);
void runtime_set_string_array_element(RuntimeState *state, const char *name, int *indices, int num_indices, const char *value);
char *runtime_get_string_array_element(RuntimeState *state, const char *name, int *indices, int num_indices);

int runtime_push_call(RuntimeState *state, int return_line);
int runtime_pop_call(RuntimeState *state);

void runtime_set_error(RuntimeState *state, int code, int line);
int runtime_get_error(RuntimeState *state);
int runtime_get_error_line(RuntimeState *state);
void runtime_clear_error(RuntimeState *state);
void runtime_clear_all(RuntimeState *state);
void runtime_set_error_handler(RuntimeState *state, int line);
int runtime_get_error_handler(RuntimeState *state);
void runtime_set_in_error_handler(RuntimeState *state, int in_handler);
int runtime_is_in_error_handler(RuntimeState *state);

double runtime_random(RuntimeState *state);
void runtime_randomize(RuntimeState *state, int seed);
double runtime_get_last_rnd(RuntimeState *state);

void runtime_set_trace(RuntimeState *state, int trace_on);
int runtime_get_trace(RuntimeState *state);
VarType runtime_get_variable_type(RuntimeState *state, const char *name);

void runtime_set_def_range(RuntimeState *state, VarType type, char start_letter, char end_letter);

/* DATA/READ support */
void runtime_data_reset(RuntimeState *state);
void runtime_data_reset_to_line(RuntimeState *state, int line_number);
void runtime_data_clear(RuntimeState *state);
void runtime_data_start_segment(RuntimeState *state, int line_number);
void runtime_data_add_number(RuntimeState *state, double value);
void runtime_data_add_string(RuntimeState *state, const char *value);
int runtime_data_read(RuntimeState *state, VarType *out_type, double *out_num, char **out_str);

/* File I/O support */
int runtime_open_file(RuntimeState *state, int handle, const char *filename, const char *mode);
void runtime_close_file(RuntimeState *state, int handle);
int runtime_file_eof(RuntimeState *state, int handle);
long runtime_file_loc(RuntimeState *state, int handle);
long runtime_file_lof(RuntimeState *state, int handle);
int runtime_file_get(RuntimeState *state, int handle, int *out_byte);
int runtime_file_put(RuntimeState *state, int handle, int byte_val);
FILE *runtime_get_file(RuntimeState *state, int handle);

/* Memory/register support (POKE/PEEK/USR) */
void runtime_poke(RuntimeState *state, int addr, int value);
int runtime_peek(RuntimeState *state, int addr);
void runtime_set_usr_address(RuntimeState *state, int addr);
int runtime_get_usr_address(RuntimeState *state);
void runtime_set_reg_a(RuntimeState *state, int value);
void runtime_set_reg_b(RuntimeState *state, int value);
int runtime_get_reg_a(RuntimeState *state);
int runtime_get_reg_b(RuntimeState *state);
int runtime_get_var_address(RuntimeState *state, const char *name);

/* SAVE callback support - for saving program from within execution */
typedef int (*SaveCallback)(const char *filename);
void runtime_set_save_callback(RuntimeState *state, SaveCallback callback);
SaveCallback runtime_get_save_callback(RuntimeState *state);

/* DELETE callback support - for deleting lines from program */
typedef int (*DeleteCallback)(int start_line, int end_line);
void runtime_set_delete_callback(RuntimeState *state, DeleteCallback callback);
DeleteCallback runtime_get_delete_callback(RuntimeState *state);

/* MERGE callback support - for merging program files */
typedef int (*MergeCallback)(const char *filename);
void runtime_set_merge_callback(RuntimeState *state, MergeCallback callback);
MergeCallback runtime_get_merge_callback(RuntimeState *state);

int runtime_get_last_entered_line(RuntimeState *state);
void runtime_set_last_entered_line(RuntimeState *state, int line_number);

/* User-defined function support */
struct ASTExpr; /* Forward declaration */

int runtime_define_function(RuntimeState *state, const char *name,
                            const char **parameters, int num_parameters,
                            struct ASTExpr *body);
struct ASTExpr *runtime_get_function_body(RuntimeState *state, const char *name);
const char **runtime_get_function_params(RuntimeState *state, const char *name);
int runtime_get_function_param_count(RuntimeState *state, const char *name);

/* STOP/CONT support */
void runtime_set_stop_state(RuntimeState *state, int line_number);
void runtime_clear_stop_state(RuntimeState *state);
int runtime_is_stopped(RuntimeState *state);
int runtime_get_stop_line(RuntimeState *state);

/* DO..LOOP stack support */
int runtime_push_do_loop(RuntimeState *state, int do_line, int condition_type, void *condition);
int runtime_pop_do_loop(RuntimeState *state, int *out_loop_line);
int runtime_get_do_loop_depth(RuntimeState *state);
int runtime_get_current_do_line(RuntimeState *state);
int runtime_set_current_loop_line(RuntimeState *state, int loop_line);
int runtime_get_current_condition_type(RuntimeState *state);
void *runtime_get_current_condition(RuntimeState *state);

/* Variable iteration for debugging */
int runtime_get_var_count(RuntimeState *state);
typedef struct
{
    const char *name;
    int is_string;
    int is_array;
    double numeric_value;
    char *string_value;
} RuntimeVar;
int runtime_get_var_by_index(RuntimeState *state, int index, RuntimeVar *out_var);

#endif /* RUNTIME_H */
