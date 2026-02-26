#ifndef COMPAT_H
#define COMPAT_H

/*
 * TRS-80 Level II BASIC Compatibility Checker
 *
 * Tracks violations of strict TRS-80 BASIC syntax to help users
 * write programs compatible with original TRS-80 hardware.
 */

typedef enum
{
    COMPAT_ARRAY_WITHOUT_DIM, /* Array used without DIM statement */
    COMPAT_MODERN_KEYWORD,    /* Non-TRS-80 keyword (SLEEP, DEFINT, etc.) */
    COMPAT_ERROR_HANDLING,    /* ON ERROR GOTO, RESUME, etc. */
    COMPAT_LONG_LINE,         /* Line exceeds TRS-80 limit */
    COMPAT_EXTENDED_FUNCTION, /* Non-TRS-80 function */
    COMPAT_FILE_MODE,         /* Advanced file I/O not in TRS-80 */
    COMPAT_LINE_NUMBER_RANGE  /* Line number outside 0-65529 */
} CompatViolationType;

typedef struct CompatViolation
{
    CompatViolationType type;
    int line_number;
    char *description;
    struct CompatViolation *next;
} CompatViolation;

typedef struct
{
    int strict_mode;             /* 1 = enforce TRS-80 compatibility */
    CompatViolation *violations; /* Linked list of violations */
    int violation_count;
} CompatState;

/* Global compatibility state */
extern CompatState *g_compat_state;

/* Initialize compatibility checking */
CompatState *compat_init(int strict_mode);

/* Free compatibility state */
void compat_free(CompatState *state);

/* Record a compatibility violation */
void compat_record_violation(CompatState *state, CompatViolationType type,
                             int line_number, const char *description);

/* Check if in strict mode */
int compat_is_strict(CompatState *state);

/* Print all violations (for RUN CHECK) */
void compat_print_violations(CompatState *state);

/* Clear all violations */
void compat_clear_violations(CompatState *state);

/* Check if keyword is TRS-80 compatible */
int compat_is_trs80_keyword(const char *keyword);

/* Check if function is TRS-80 compatible */
int compat_is_trs80_function(const char *function);

/* Scan program for arrays without DIM statements (internal use in main.c) */
void compat_check_program_arrays(void *program_ptr, CompatState *state);

#endif /* COMPAT_H */
