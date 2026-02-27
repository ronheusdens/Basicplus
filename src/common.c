#include "common.h"
#include <stdarg.h>

/*
 * Memory allocation wrappers with error checking
 */

void *xmalloc(size_t size)
{
    if (size == 0)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xmalloc: attempted zero-size allocation");
    }
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xmalloc: out of memory (%zu bytes)", size);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size)
{
    if (count == 0 || size == 0)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xcalloc: attempted zero-size allocation");
    }
    void *ptr = calloc(count, size);
    if (ptr == NULL)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xcalloc: out of memory (%zu x %zu bytes)", count, size);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    if (size == 0)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xrealloc: attempted zero-size allocation");
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        error_exit(ERR_OUT_OF_MEMORY, "xrealloc: out of memory (%zu bytes)", size);
    }
    return new_ptr;
}

char *xstrdup(const char *str)
{
    if (str == NULL)
    {
        return NULL;
    }
    size_t len = strlen(str);
    char *copy = xmalloc(len + 1);
    strcpy(copy, str);
    return copy;
}

/*
 * Platform detection functions
 */

const char *platform_name(void)
{
#if PLATFORM_MACOS
    return "macOS";
#elif PLATFORM_LINUX
    return "Linux";
#else
    return "Unknown";
#endif
}

const char *arch_name(void)
{
#if ARCH_ARM64
    return "ARM64";
#elif ARCH_X86_64
    return "x86_64";
#else
    return "Unknown";
#endif
}

/*
 * Error handling utilities
 */

void error_exit(int code, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR [%d]: ", code);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(code);
}
