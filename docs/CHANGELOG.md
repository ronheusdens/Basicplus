# Changelog

## Version 1.6.0 - Type Declaration Statements (2026-01-19)

### Added
- **Type Declaration Statements** - Microsoft BASIC / TRS-80 compatible
  - `DEFINT letter[-letter]` - Declare integer type variables
  - `DEFSNG letter[-letter]` - Declare single precision variables (default)
  - `DEFDBL letter[-letter]` - Declare double precision variables  
  - `DEFSTR letter[-letter]` - Declare string type variables
  - Support for single letters: `DEFINT I`
  - Support for letter ranges: `DEFINT I-N`
  - Support for multiple ranges: `DEFINT A-C, X-Z`
  - String suffix ($) overrides type declarations
  - Type declarations apply to arrays
  - Variables can be redeclared with different types

### Type Declaration Features
- **Letter-Based Types**: Set default type for variables by first letter
- **Range Support**: Declare types for letter ranges (e.g., I-N for integers)
- **Multiple Ranges**: Combine multiple ranges in one statement
- **Type Precedence**: $ suffix always creates string, overriding declarations
- **Array Support**: Type declarations apply to array variables
- **Redeclaration**: Types can be changed with subsequent DEFxxx statements

### Technical Implementation
- Added `VarType` enum for type tracking (INTEGER, SINGLE, DOUBLE, STRING)
- Added `letter_types[26]` array for letter-to-type mapping
- Modified variable creation in LET, READ, DIM to respect DEFxxx
- Updated PRINT statement to handle DEFSTR'd variables
- All numeric types stored internally as `double` for compatibility

### Example Usage
```basic
10 DEFINT I-N          ' Integer loop counters
20 DEFDBL X-Z          ' High precision coordinates
30 DEFSTR S            ' String variables
40 FOR I = 1 TO 10
50   LET X = 3.141592653589793
60   LET S = "Result"
70   PRINT I; X; S
80 NEXT I
```

### Documentation
- Added `tests/DEFTYPE_REFERENCE.md` - Complete reference guide
- Added `tests/test_deftype.bas` - Comprehensive test suite
- Updated `QUICK_REFERENCE.md` with DEFxxx commands

## Version 1.5.0 - Error Handling System (2026-01-19)

### Added
- **Complete Error Handling System** - TRS-80 Disk BASIC / Microsoft BASIC compatible
  - `ON ERROR GOTO n` - Set error handler at line n
  - `ON ERROR GOTO 0` - Disable error handler
  - `ERROR code` - Simulate/trigger an error with code
  - `RESUME` - Return to line where error occurred
  - `RESUME n` - Resume at specific line number
  - `RESUME NEXT` - Resume at line after error
  - `ERR` - Function returning last error code
  - `ERL` - Function returning line number where error occurred

### Error Handling Features
- **Automatic Error Trapping**: When ON ERROR GOTO is active, errors are caught automatically
- **Error Information**: ERR and ERL provide error code and line number
- **Flexible Resume**: Resume execution at error line, next line, or specific line
- **Nested Protection**: Prevents recursive error handler calls
- **Manual Errors**: ERROR statement allows custom error codes for application logic

### Example Usage
```basic
10 ON ERROR GOTO 1000
20 PRINT "Protected code"
30 ERROR 42
40 PRINT "This won't print"
50 END
1000 PRINT "Error"; ERR; "at line"; ERL
1010 RESUME NEXT
```

## Version 1.4.0 - Additional Math Functions (2026-01-19)

### Added
- **ATN(x)**: Arctangent function - returns angle in radians
- **FIX(x)**: Truncate towards zero (different from INT which floors)
- **CINT(x)**: Round to nearest integer (banker's rounding)
- **CDBL(x)**: Convert to double (no-op, already double precision)
- **CSNG(x)**: Convert to single (no-op, treated as double)
- **RANDOM**: Alternative to RND for random number generation

### Function Comparison
| Function | Behavior | Example |
|----------|----------|---------|
| INT(3.7) | Floor (down) | 3 |
| FIX(3.7) | Truncate towards 0 | 3 |
| CINT(3.7) | Round to nearest | 4 |
| INT(-3.7) | Floor (down) | -4 |
| FIX(-3.7) | Truncate towards 0 | -3 |
| CINT(-3.7) | Round to nearest | -4 |

### Complete Function List
Now supports all standard TRS-80 Model 3/4 Disk BASIC math functions:
- **Basic**: ABS, SGN, SQR, INT, FIX, CINT
- **Trigonometric**: SIN, COS, TAN, ATN
- **Exponential**: EXP, LOG
- **Random**: RND, RANDOM
- **Type conversion**: CDBL, CSNG

## Version 1.3.0 - Error Reporting (2026-01-18)

### Added
- **Unknown Command Detection**: Interpreter now reports errors for unrecognized commands
  - Reports `?SYNTAX ERROR - UNKNOWN COMMAND: <name>` for invalid identifiers
  - Helps catch typos and unsupported commands
  - Program continues execution after error (non-fatal)
  - Works in both program execution and interactive mode

### Example
```basic
10 PRINT "Hello"
20 WRONGCMD
30 PRINT "Still runs"
```
Output:
```
Hello
?SYNTAX ERROR - UNKNOWN COMMAND: WRONGCMD
Still runs
```

## Version 1.2.0 - CLS Command (2026-01-17)

### Added
- **CLS Command**: Clear screen command from TRS-80 Disk BASIC
  - Usage: `CLS` clears the terminal screen
  - Works in both program lines and interactive mode
  - Uses ANSI escape codes for terminal clearing

## Version 1.1.1 - Bug Fix (2026-01-17)

### Fixed
- **LOAD/SAVE Syntax Error**: Fixed "SYNTAX ERROR - SAVE/LOAD requires filename" bug that occurred in interactive mode
  - **Root Cause**: Position pointer was being incremented twice - once in `exec_stmt()` after reading the keyword, and again in the `KW_LOAD`/`KW_SAVE` case statements
  - **Solution**: Removed redundant `(*pos)++` from `KW_LOAD` and `KW_SAVE` cases since position is already incremented past the keyword
  - LOAD and SAVE commands now work correctly: `SAVE "test.bas"` and `LOAD "test.bas"`

## Version 1.1 - Interactive Improvements

### New Features

#### 1. Ctrl-C Signal Handling
- **Press Ctrl-C** during program execution to stop gracefully
- Returns to the `>` prompt with "** BREAK **" message
- Interpreter stays running (doesn't exit)
- Allows you to edit and re-run programs without restarting

#### 2. EXIT Command
- New `EXIT` keyword to exit the interpreter
- Works alongside existing `BYE`, `QUIT`, and `END` commands
- Usage: Type `EXIT` at the prompt

#### 3. LOAD Command
- Load BASIC programs from disk
- Syntax: `LOAD "filename.bas"`
- Clears current program before loading
- Shows "Loaded filename.bas" on success
- Shows "?FILE NOT FOUND" error if file doesn't exist
- Compatible with TRS-80 Disk BASIC syntax

#### 4. SAVE Command
- Save current program to disk
- Syntax: `SAVE "filename.bas"`
- Saves all program lines in order
- Shows "Saved filename.bas" on success
- Shows "?FILE ERROR" if cannot write
- Compatible with TRS-80 Disk BASIC syntax

#### 5. Unknown Keyword Detection
- Detects and reports unknown keywords
- Shows error: "?SYNTAX ERROR - UNKNOWN KEYWORD"
- Helps catch typos and unsupported commands

#### 6. Duplicate Line Number Handling
- When entering a line with an existing line number, the new line **replaces** the old one
- Last line entered with a given number is the one that executes
- Makes program editing more intuitive

### Usage Examples

```basic
' Save your work
> SAVE "myprogram.bas"
Saved myprogram.bas

' Load a program
> LOAD "myprogram.bas"
Loaded myprogram.bas

' Run it
> RUN
(program output...)

' Press Ctrl-C to stop if needed
** BREAK **

' Edit a line (replaces line 10)
> 10 PRINT "Updated line"

' Exit when done
> EXIT
```

### Technical Details

- Signal handling uses POSIX `sigaction()`
- File operations use standard C `fopen()/fclose()`
- Interrupt flag is `volatile sig_atomic_t` for thread safety
- LOAD/SAVE work with standard text files (one line per program line)

### Compatibility

- macOS (ARM64/Intel)
- Linux (x64)
- Compatible with TRS-80 Level II BASIC programs
- File format compatible with TRS-80 Disk BASIC
