; ARM64 machine code routines for BASIC USR calls
; Assembly file for extracting bytecode

.globl _basic_add
.align 4
_basic_add:
    ; Input: x0 = operand A, x1 = operand B
    ; Output: x0 = A + B
    add x0, x0, x1
    ret

.globl _basic_subtract
.align 4
_basic_subtract:
    ; Input: x0 = operand A, x1 = operand B
    ; Output: x0 = A - B
    sub x0, x0, x1
    ret

.globl _basic_multiply
.align 4
_basic_multiply:
    ; Input: x0 = operand A, x1 = operand B
    ; Output: x0 = A * B
    mul x0, x0, x1
    ret

.globl _basic_square
.align 4
_basic_square:
    ; Input: x0 = operand A
    ; Output: x0 = A * A
    mul x0, x0, x0
    ret

.globl _basic_negate
.align 4
_basic_negate:
    ; Input: x0 = operand A
    ; Output: x0 = -A
    neg x0, x0
    ret

.globl _basic_absolute
.align 4
_basic_absolute:
    ; Input: x0 = operand A
    ; Output: x0 = |A|
    cmp x0, #0
    b.ge abs_done
    neg x0, x0
abs_done:
    ret

.globl _basic_get_date
.align 4
_basic_get_date:
    ; Output: x0 = YYYYMMDD (20260128 for Jan 28, 2026)
    mov x0, #0x0128        ; Load immediate 0x0128 (296)
    movk x0, #0x0126, lsl #16  ; Or in upper 16 bits for 20260128
    ret
