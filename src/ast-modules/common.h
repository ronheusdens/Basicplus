#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

/* Platform detection macros */
#ifdef __APPLE__
#define PLATFORM_MACOS 1
#define PLATFORM_LINUX 0
#else
#define PLATFORM_MACOS 0
#define PLATFORM_LINUX 1
#endif

#ifdef __aarch64__
#define ARCH_ARM64 1
#define ARCH_X86_64 0
#elif defined(__arm64__)
#define ARCH_ARM64 1
#define ARCH_X86_64 0
#else
#define ARCH_ARM64 0
#define ARCH_X86_64 1
#endif

/* Configuration macros */
#define MAX_STACK_DEPTH 1024
#define MAX_FILES 32
#define MAX_VARIABLES 10000
#define MAX_ARRAYS 1000
#define MAX_DIMENSIONS 10
#define MAX_STRING_LENGTH 32768
#define MAX_LINE_LENGTH 255
#define MAX_TOKENS 8192
#define MAX_AST_NODES 16384

/* Common error codes */
#define ERR_NONE 0
#define ERR_OUT_OF_MEMORY 1
#define ERR_SYNTAX_ERROR 2
#define ERR_UNDEFINED_VARIABLE 3
#define ERR_TYPE_MISMATCH 4
#define ERR_DIVISION_BY_ZERO 5
#define ERR_ILLEGAL_QUANTITY 6
#define ERR_FILE_NOT_FOUND 7
#define ERR_FILE_IO_ERROR 8
#define ERR_STACK_OVERFLOW 9
#define ERR_UNDEFINED_LINE 10

/* Forward declarations (actual definitions in their respective headers) */

/* Variable type enumeration */
typedef enum
{
    VAR_UNDEFINED = 0,
    VAR_DOUBLE,
    VAR_INTEGER,
    VAR_STRING,
    VAR_SINGLE,
    VAR_LONG
} VarType;

/* Memory allocation utilities (with error checking) */
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *str);

/* Platform detection functions */
const char *platform_name(void);
const char *arch_name(void);

/* Error utilities */
void error_exit(int code, const char *fmt, ...);

#endif /* COMMON_H */
