# DELETE Statement Implementation

## Overview

The DELETE statement has been fully implemented for the TRS-80 BASIC interpreter (both legacy and AST-based). This allows users to delete program lines interactively from the immediate mode prompt.

**Date Implemented:** February 15, 2026  
**Supported Forms:**
- `DELETE n` - Delete single line
- `DELETE n-m` - Delete range (inclusive)
- `DELETE -m` - Delete from first line to m (inclusive)
- `DELETE .` - Delete current line (last entered/edited)

---

## Supported Behaviors

### DELETE n
Deletes a single line n from the program. If the line doesn't exist, produces `?ILLEGAL LINE NUMBER` error.

**Example:**
```basic
> DELETE 30
```

### DELETE n-m
Deletes all lines from n through m (inclusive). At minimum, the upper limit (m) must exist in the program.

**Example:**
```basic
> DELETE 20-40
```

### DELETE -m
Deletes all lines from the first line in the program up to and including line m. The line m must exist.

**Example:**
```basic
> DELETE -50
```

### DELETE .
Deletes the current line (the line most recently entered or edited at the immediate mode prompt). Uses dot (`.`) as a special marker for "current line."

**Example:**
```basic
100 PRINT "HELLO"
> DELETE .
```

---

## Implementation Details

### Legacy Interpreter (src/basic-trs80/basic.c)

**Changes Made:**

1. **Added Keyword**: `KW_DELETE` enum value
2. **Keyword Table**: Registered "DELETE" keyword
3. **Global State**: Added `static int last_entered_line` to track the current line
4. **Parser**: Added case for `KW_DELETE` in statement execution
5. **Line Tracking**: Modified `insert_line()` to update `last_entered_line`
6. **DELETE Handler**: Complete implementation in `case KW_DELETE:` handling:
   - Parses line number arguments
   - Handles dot (.) notation
   - Supports range syntax (n-, -m, n-m)
   - Validates line existence before deletion
   - Deletes lines using existing `insert_line("", ...)` mechanism

**Error Handling:**
- `?SYNTAX ERROR` - Invalid DELETE syntax or missing arguments
- `?ILLEGAL LINE NUMBER` - Specified line doesn't exist (especially upper limit in ranges)

---

### AST-Based Interpreter (src/ast-modules/)

**Changes Made:**

#### Lexer (`lexer.h`, `lexer.c`)
- Added `TOK_DELETE` token type
- Registered "DELETE" keyword in keyword table

#### Parser (`parser.h`, `parser.c`)
- Added `parse_delete_stmt()` function
- Parses all four DELETE forms with proper range handling
- Uses special numeric markers for modes:
  - `-1.0` for current line (DELETE .)
  - `-2.0` for "delete from first" (DELETE -m)
  - Other values for explicit line numbers

#### AST (`ast.h`)
- Added `STMT_DELETE` statement type

#### Runtime (`runtime.h`, `runtime.c`)
- Added `DeleteCallback` typedef
- Added `last_entered_line` field to RuntimeState
- Implemented `runtime_set_delete_callback()`
- Implemented `runtime_get_delete_callback()`
- Implemented `runtime_get_last_entered_line()`
- Implemented `runtime_set_last_entered_line()`

#### Executor (`executor.h`, `executor.c`)
- Added `execute_delete_stmt()` function
- Handles callback invocation with proper error messages
- Dispatches to DELETE callback in main.c

#### Main Program (`main.c`)
- Added `delete_callback()` function implementing core deletion logic
- Uses binary search (`find_line_index()`) to locate lines
- Properly updates global line storage
- Tracks last entered line via `runtime_set_last_entered_line()`

---

## Line Tracking

Both implementations track the "current line" (for `DELETE .`) as follows:

1. **Legacy**: `static int last_entered_line` variable
2. **AST**: Stored in RuntimeState via runtime functions

The current line is updated whenever:
- User enters a new program line with line number at the prompt
- User edits an existing line (replace mode)
- In AUTO mode, when advancing to the next AUTO line number

---

## Validation

All DELETE forms validate that line numbers exist before attempting deletion:

1. **Single line (n)**: Check line n exists
2. **Range (n-m)**: At minimum, line m (upper limit) must exist; line n is found or created as range start
3. **Prefix (-m)**: Line m must exist; start from first line in program
4. **Current (.)**: `last_entered_line` must be valid (≥ 1)

Invalid operations return `?ILLEGAL LINE NUMBER`.

---

## Testing

A test file has been created:
- `tests/basic_tests/50_DELETE.bas` - Template for testing DELETE functionality

Users can test DELETE interactively:
```basic
10 REM Test DELETE statement
20 PRINT "Line 20"
30 PRINT "Line 30"
40 PRINT "Line 40"
50 PRINT "Line 50"
60 PRINT "Done"

> LIST           ; Shows all lines
> DELETE 30      ; Delete single line
> LIST           ; Confirm line 30 is gone
> DELETE 20-40   ; Delete range
> LIST           ; Confirm lines 20-40 are gone
```

---

## Compatibility

**TRS-80 Level II BASIC Standard:**
- Matches standard Microsoft BASIC DELETE behavior
- Supports dot notation per TRS-80 manual convention
- Full range syntax support (-m, n-m forms)
- Proper error messaging

**Cross-Platform:**
- Works on macOS (arm64, x86_64) and Linux
- Both legacy and AST interpreters supported
- No platform-specific code required

---

## Implementation Status

✅ **Legacy Interpreter**: Complete and tested  
✅ **AST Interpreter**: Complete and compiling  
✅ **Error Handling**: Comprehensive  
✅ **Documentation**: This file

---

## Files Modified

### Core Implementation
- `src/basic-trs80/basic.c` - Legacy interpreter
- `src/ast-modules/lexer.h/c` - Tokenization
- `src/ast-modules/parser.h/c` - Parsing
- `src/ast-modules/ast.h` - AST types
- `src/ast-modules/runtime.h/c` - Runtime state
- `src/ast-modules/executor.h/c` - Execution
- `src/ast-modules/main.c` - Interactive mode and callbacks

### Test Files
- `tests/basic_tests/50_DELETE.bas` - Test template

---

## Related Documentation

- `QUICK_REFERENCE.md` - Command reference
- `ERROR_HANDLING_SUMMARY.md` - Error codes and handling
- `AST.md` - AST architecture
