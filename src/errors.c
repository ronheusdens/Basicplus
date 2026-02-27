#include "errors.h"

const char *error_message(int error_code)
{
    switch (error_code)
    {
    case BASIC_ERR_NEXT_WITHOUT_FOR:
        return "NEXT without FOR";
    case BASIC_ERR_SYNTAX_ERROR:
        return "Syntax error";
    case BASIC_ERR_RETURN_WITHOUT_GOSUB:
        return "Return without GOSUB";
    case BASIC_ERR_OUT_OF_DATA:
        return "Out of data";
    case BASIC_ERR_ILLEGAL_QUANTITY:
        return "Illegal quantity";
    case BASIC_ERR_OVERFLOW:
        return "Overflow";
    case BASIC_ERR_OUT_OF_MEMORY:
        return "Out of memory";
    case BASIC_ERR_UNDEFINED_LINE:
        return "Undefined line";
    case BASIC_ERR_SUBSCRIPT_OUT_OF_RANGE:
        return "Subscript out of range";
    case BASIC_ERR_REDIMENSIONED_ARRAY:
        return "Redimensioned array";
    case BASIC_ERR_DIVISION_BY_ZERO:
        return "Division by zero";
    case BASIC_ERR_ILLEGAL_DIRECT:
        return "Illegal direct";
    case BASIC_ERR_TYPE_MISMATCH:
        return "Type mismatch";
    case BASIC_ERR_OUT_OF_STRING_SPACE:
        return "Out of string space";
    case BASIC_ERR_STRING_TOO_LONG:
        return "String too long";
    case BASIC_ERR_STRING_FORMULA_TOO_COMPLEX:
        return "String formula too complex";
    case BASIC_ERR_CANT_CONTINUE:
        return "Can't continue";
    case BASIC_ERR_NO_RESUME:
        return "No RESUME";
    case BASIC_ERR_RESUME_WITHOUT_ERROR:
        return "RESUME without error";
    case BASIC_ERR_UNPRINTABLE_ERROR:
        return "Unprintable error";
    case BASIC_ERR_MISSING_OPERAND:
        return "Missing operand";
    case BASIC_ERR_BAD_FILE_DATA:
        return "Bad file data";
    case BASIC_ERR_DISK_BASIC:
        return "Disk BASIC";
    default:
        return "Unknown error";
    }
}

void error_print(int error_code, int line_number)
{
    fprintf(stderr, "?%s IN %d\n", error_message(error_code), line_number);
}
