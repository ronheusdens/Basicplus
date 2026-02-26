#ifndef ARM64_ROUTINES_H
#define ARM64_ROUTINES_H

#include <stddef.h>

/* ============================================================
 * ARM64 Machine Code Routines for BASIC USR Calls
 *
 * Each routine can be called via USR(address) or USR() with DEFUSR
 * Parameters passed in x0 (A register) and x1 (B register)
 * Return value in x0
 * ============================================================ */

/* basic_add: x0 = x0 + x1 */
static const unsigned char arm64_add_code[] = {
    0x00, 0x00, 0x01, 0x8b, /* add x0, x0, x1 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_add_size = sizeof(arm64_add_code);
#define ARM64_ADD_ADDR 1000

/* basic_subtract: x0 = x0 - x1 */
static const unsigned char arm64_subtract_code[] = {
    0x00, 0x00, 0x01, 0xcb, /* sub x0, x0, x1 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_subtract_size = sizeof(arm64_subtract_code);
#define ARM64_SUBTRACT_ADDR 1100

/* basic_multiply: x0 = x0 * x1 */
static const unsigned char arm64_multiply_code[] = {
    0x00, 0x7c, 0x01, 0x9b, /* mul x0, x0, x1 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_multiply_size = sizeof(arm64_multiply_code);
#define ARM64_MULTIPLY_ADDR 1200

/* basic_square: x0 = x0 * x0 */
static const unsigned char arm64_square_code[] = {
    0x00, 0x7c, 0x00, 0x9b, /* mul x0, x0, x0 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_square_size = sizeof(arm64_square_code);
#define ARM64_SQUARE_ADDR 1300

/* basic_negate: x0 = -x0 */
static const unsigned char arm64_negate_code[] = {
    0xe0, 0x03, 0x00, 0xcb, /* neg x0, x0 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_negate_size = sizeof(arm64_negate_code);
#define ARM64_NEGATE_ADDR 1400

/* basic_absolute: x0 = |x0| (absolute value) */
static const unsigned char arm64_absolute_code[] = {
    0x1f, 0x00, 0x00, 0xf1, /* cmp x0, #0 */
    0x4a, 0x00, 0x00, 0x54, /* b.ge abs_done (skip next instruction if >= 0) */
    0xe0, 0x03, 0x00, 0xcb, /* neg x0, x0 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_absolute_size = sizeof(arm64_absolute_code);
#define ARM64_ABSOLUTE_ADDR 1500

/* basic_get_date: x0 = 20260128 (YYYYMMDD for Jan 28, 2026) */
static const unsigned char arm64_get_date_code[] = {
    0x00, 0x25, 0x80, 0xd2, /* mov x0, #0x128 (296 decimal) */
    0xc0, 0x24, 0xa0, 0xf2, /* movk x0, #0x126, lsl #16 */
    0xc0, 0x03, 0x5f, 0xd6  /* ret */
};
static const size_t arm64_get_date_size = sizeof(arm64_get_date_code);
#define ARM64_GET_DATE_ADDR 1600

/* ============================================================
 * Routine Registry
 * ============================================================ */

typedef struct
{
    const char *name;
    const unsigned char *code;
    size_t size;
    int address;
    const char *description;
} ARM64Routine;

static const ARM64Routine arm64_routines[] = {
    {"add",
     arm64_add_code,
     arm64_add_size,
     ARM64_ADD_ADDR,
     "Addition: x0 = x0 + x1"},
    {"sub",
     arm64_subtract_code,
     arm64_subtract_size,
     ARM64_SUBTRACT_ADDR,
     "Subtraction: x0 = x0 - x1"},
    {"mul",
     arm64_multiply_code,
     arm64_multiply_size,
     ARM64_MULTIPLY_ADDR,
     "Multiplication: x0 = x0 * x1"},
    {"square",
     arm64_square_code,
     arm64_square_size,
     ARM64_SQUARE_ADDR,
     "Square: x0 = x0 * x0"},
    {"neg",
     arm64_negate_code,
     arm64_negate_size,
     ARM64_NEGATE_ADDR,
     "Negate: x0 = -x0"},
    {"abs",
     arm64_absolute_code,
     arm64_absolute_size,
     ARM64_ABSOLUTE_ADDR,
     "Absolute value: x0 = |x0|"},
    {"date",
     arm64_get_date_code,
     arm64_get_date_size,
     ARM64_GET_DATE_ADDR,
     "Get date: x0 = 20260128 (YYYYMMDD)"},
    {NULL, NULL, 0, 0, NULL} /* Sentinel */
};

#endif /* ARM64_ROUTINES_H */
