# Error Handling System Reference

**TRS-80 Level II BASIC Interpreter**  
**Version**: 1.5.0  
**Date**: 2026-01-19  
**Status**: ✅ FULLY IMPLEMENTED

---

## Overview

The interpreter now includes a complete error handling system compatible with TRS-80 Disk BASIC and Microsoft BASIC. This allows programs to trap and handle errors gracefully instead of crashing.

---

## Commands and Functions

### ON ERROR GOTO n

Sets up an error handler at line number `n`. When an error occurs, execution jumps to line `n`.

**Syntax:**
```basic
ON ERROR GOTO line_number
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 PRINT "Protected code"
30 ERROR 10
40 PRINT "This won't print"
1000 PRINT "Error handler"
1010 END
```

### ON ERROR GOTO 0

Disables the error handler. Subsequent errors will stop program execution.

**Syntax:**
```basic
ON ERROR GOTO 0
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 PRINT "Handler active"
30 ON ERROR GOTO 0
40 PRINT "Handler disabled"
```

### ERROR code

Triggers an error with the specified error code. Useful for simulating errors or implementing custom error conditions.

**Syntax:**
```basic
ERROR error_code
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 ERROR 42
30 END
1000 PRINT "Error code:"; ERR
1010 END
```

### RESUME

Returns to the line where the error occurred. Use this to retry the operation.

**Syntax:**
```basic
RESUME
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 LET X = 1 / 0
30 PRINT "Success"
40 END
1000 PRINT "Error, retrying..."
1010 RESUME
```

### RESUME line_number

Resumes execution at a specific line number.

**Syntax:**
```basic
RESUME line_number
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 ERROR 10
30 PRINT "Skipped"
40 PRINT "Executed"
50 END
1000 PRINT "Resuming at line 40"
1010 RESUME 40
```

### RESUME NEXT

Resumes execution at the line immediately after where the error occurred.

**Syntax:**
```basic
RESUME NEXT
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 PRINT "Before error"
30 ERROR 10
40 PRINT "After error"
50 END
1000 PRINT "Error caught"
1010 RESUME NEXT
```

### ERR

Function that returns the error code of the last error.

**Syntax:**
```basic
ERR
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 ERROR 42
1000 PRINT "Error code:"; ERR
1010 END
```

### ERL

Function that returns the line number where the last error occurred.

**Syntax:**
```basic
ERL
```

**Example:**
```basic
10 ON ERROR GOTO 1000
20 ERROR 10
1000 PRINT "Error at line:"; ERL
1010 END
```

---

## Error Codes

You can use any integer error code with the ERROR statement. Common conventions:

| Code | Meaning |
|------|---------|
| 1-10 | Application-specific errors |
| 11-20 | Input/validation errors |
| 50-99 | Recoverable errors |
| 100+ | Critical errors |

**Note:** The interpreter doesn't enforce specific error codes - you define their meaning.

---

## Complete Examples

### Example 1: Basic Error Trapping

```basic
10 REM Basic error handling
20 ON ERROR GOTO 1000
30 PRINT "Starting..."
40 ERROR 10
50 PRINT "This line is skipped"
60 PRINT "Program continues"
70 END
1000 REM Error handler
1010 PRINT "Error"; ERR; "at line"; ERL
1020 RESUME NEXT
```

**Output:**
```
Starting...
Error 10 at line 40
Program continues
```

### Example 2: Division by Zero Protection

```basic
10 ON ERROR GOTO 5000
20 INPUT "Enter dividend: "; A
30 INPUT "Enter divisor: "; B
40 LET C = A / B
50 PRINT "Result:"; C
60 END
5000 REM Division error handler
5010 IF ERR = 11 THEN PRINT "Cannot divide by zero!"
5020 PRINT "Please try again"
5030 RESUME 20
```

### Example 3: Multiple Error Types

```basic
10 ON ERROR GOTO 9000
20 PRINT "Choose operation:"
30 PRINT "1=Divide, 2=Sqrt, 3=Log"
40 INPUT N
50 IF N = 1 THEN ERROR 1
60 IF N = 2 THEN ERROR 2
70 IF N = 3 THEN ERROR 3
80 GOTO 20
90 END
9000 REM Error dispatcher
9010 IF ERR = 1 THEN GOSUB 9100: RESUME 20
9020 IF ERR = 2 THEN GOSUB 9200: RESUME 20
9030 IF ERR = 3 THEN GOSUB 9300: RESUME 20
9040 PRINT "Unknown error": END
9100 PRINT "Division selected": RETURN
9200 PRINT "Square root selected": RETURN
9300 PRINT "Logarithm selected": RETURN
```

### Example 4: Retry Logic

```basic
10 LET TRIES = 0
20 ON ERROR GOTO 8000
30 LET TRIES = TRIES + 1
40 PRINT "Attempt"; TRIES
50 IF TRIES < 3 THEN ERROR 99
60 PRINT "Success!"
70 END
8000 REM Retry handler
8010 IF TRIES >= 3 THEN RESUME 60
8020 PRINT "  Failed, retrying..."
8030 RESUME 30
```

**Output:**
```
Attempt 1
  Failed, retrying...
Attempt 2
  Failed, retrying...
Attempt 3
Success!
```

### Example 5: Cleanup on Error

```basic
10 ON ERROR GOTO 7000
20 PRINT "Opening resources..."
30 LET RESOURCE_OPEN = 1
40 PRINT "Processing..."
50 ERROR 50
60 PRINT "Closing resources..."
70 LET RESOURCE_OPEN = 0
80 END
7000 REM Error cleanup
7010 PRINT "Error occurred!"
7020 IF RESOURCE_OPEN = 1 THEN PRINT "Cleaning up..."
7030 LET RESOURCE_OPEN = 0
7040 PRINT "Cleanup complete"
7050 END
```

---

## Best Practices

### 1. Always Clean Up Resources

```basic
10 ON ERROR GOTO 9000
20 REM Open file/resource
30 REM Process
40 REM Close file/resource
50 END
9000 REM Close resources in error handler too
9010 REM Cleanup code
9020 END
```

### 2. Use Descriptive Error Codes

```basic
10 LET ERR_FILE_NOT_FOUND = 1
20 LET ERR_INVALID_INPUT = 2
30 LET ERR_OUT_OF_RANGE = 3
40 REM Use these constants
50 IF X < 0 THEN ERROR ERR_OUT_OF_RANGE
```

### 3. Log Errors for Debugging

```basic
1000 REM Error handler
1010 PRINT "ERROR LOG:"
1020 PRINT "  Code:"; ERR
1030 PRINT "  Line:"; ERL
1040 PRINT "  Time: [timestamp]"
1050 RESUME NEXT
```

### 4. Avoid Infinite Loops

```basic
10 LET ERROR_COUNT = 0
1000 REM Error handler
1010 LET ERROR_COUNT = ERROR_COUNT + 1
1020 IF ERROR_COUNT > 10 THEN PRINT "Too many errors": END
1030 RESUME
```

### 5. Disable Handler When Not Needed

```basic
10 ON ERROR GOTO 1000
20 REM Protected code
30 ON ERROR GOTO 0
40 REM Normal code (errors will stop program)
```

---

## Error Handler Structure

### Simple Handler

```basic
1000 REM Error handler
1010 PRINT "Error:"; ERR
1020 RESUME NEXT
```

### Comprehensive Handler

```basic
9000 REM === ERROR HANDLER ===
9010 PRINT "ERROR OCCURRED"
9020 PRINT "  Code:"; ERR
9030 PRINT "  Line:"; ERL
9040 REM
9050 REM Check error type
9060 IF ERR = 1 THEN GOSUB 9500: RESUME NEXT
9070 IF ERR = 2 THEN GOSUB 9600: RESUME 100
9080 REM
9090 REM Unknown error - stop
9100 PRINT "  Unknown error - stopping"
9110 END
9120 REM
9500 REM Handle error type 1
9510 PRINT "  Type 1 handled"
9520 RETURN
9530 REM
9600 REM Handle error type 2
9610 PRINT "  Type 2 handled"
9620 RETURN
```

---

## Limitations and Notes

### 1. No Nested Error Handlers

The interpreter prevents recursive error handler calls. If an error occurs within an error handler, the program will stop.

```basic
1000 REM Error handler
1010 ERROR 99  ' This will stop the program, not call handler again
```

### 2. RESUME Without Error

Using RESUME outside an error handler produces an error:

```basic
10 RESUME NEXT  ' Error: RESUME WITHOUT ERROR
```

### 3. Error Handler Scope

The error handler is global - it applies to all subsequent code until disabled with `ON ERROR GOTO 0`.

### 4. Line Number Required

`ON ERROR GOTO` requires a valid line number. The line must exist in the program.

---

## Compatibility

### TRS-80 Disk BASIC

✅ **Fully Compatible** with TRS-80 Model 3/4 Disk BASIC error handling:
- ON ERROR GOTO n
- ERROR code
- RESUME / RESUME n / RESUME NEXT
- ERR and ERL functions

### Microsoft BASIC Variants

✅ **Compatible** with:
- GW-BASIC
- QuickBASIC
- IBM BASIC
- Microsoft BASIC-80

### Differences from Some BASICs

⚠️ **RESUME 0** - In this interpreter, `RESUME 0` returns to the error line (same as `RESUME`). Some BASICs treat 0 specially.

⚠️ **Error Codes** - Error codes are user-defined. Some BASICs have predefined system error codes.

---

## Testing

### Test Files

1. **`test_error_handling_system.bas`** - Complete system test
   - Tests all error handling commands
   - Verifies ERR and ERL functions
   - Tests RESUME variants

2. **`test_error_examples.bas`** - Practical examples
   - Division by zero protection
   - Multiple error types
   - Retry logic
   - Real-world patterns

### Running Tests

```bash
./build/bin/basic-trs80 tests/test_error_handling_system.bas
./build/bin/basic-trs80 tests/test_error_examples.bas
```

---

## Implementation Details

### Global State

```c
static int error_handler_line = 0;    /* ON ERROR GOTO target */
static int last_error_code = 0;       /* ERR value */
static int last_error_line = 0;       /* ERL value */
static int in_error_handler = 0;      /* Recursion prevention */
```

### Error Flow

1. Error occurs (via ERROR statement)
2. Set `last_error_code` and `last_error_line`
3. If `error_handler_line > 0`, jump to handler
4. Otherwise, display error and stop
5. In handler, ERR and ERL return stored values
6. RESUME clears `in_error_handler` flag and returns

---

## Quick Reference

| Command | Purpose |
|---------|---------|
| `ON ERROR GOTO n` | Set error handler |
| `ON ERROR GOTO 0` | Disable handler |
| `ERROR code` | Trigger error |
| `RESUME` | Return to error line |
| `RESUME n` | Resume at line n |
| `RESUME NEXT` | Resume after error |
| `ERR` | Get error code |
| `ERL` | Get error line |

---

**Documentation Version**: 1.0  
**Last Updated**: 2026-01-19  
**Interpreter Version**: 1.5.0  
**Feature Status**: ✅ Production Ready
