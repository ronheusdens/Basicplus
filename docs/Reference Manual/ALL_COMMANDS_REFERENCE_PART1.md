# TRS-80 BASIC Command Reference (A–G)

This is Part 1 of the complete command reference. For quick navigation, see the Table of Contents below.

## Table of Contents

| Command | Description |
|---------|-------------|
| [ABS](#abs) | Absolute value |
| [LN](#ln) | Natural logarithm (base e) |
| [EXP](#exp) | Exponential function (e^x) |
| [AND](#and) | Logical AND operation |
| [APPEND](#append) | Open file for appending |
| [AS](#as) | File number in I/O |
| [AT](#at) | Screen position for output |
| [BEEP](#beep) | Produce a beep sound |
| [BYE](#bye) | Exit interpreter (alias) |
| [CASE](#case) | Multi-way branching statement |
| [CIRCLE](#circle) | Draw a circle (graphics) |
| [CLEAR](#clear) | Clear variables and memory |
| [CLOSE](#close) | Close a file |
| [CLS](#cls) | Clear the screen |
| [COLOR](#color) | Set foreground color |
| [DATA_READ_RESTORE](#data_read_restore) | Data statements and reading |
| [DEFDBL](#defdbl) | Default double variables |
| [DEFINT](#defint) | Default integer variables |
| [DEFSNG](#defsng) | Default single variables |
| [DEFSTR](#defstr) | Default string variables |
| [DELETE](#delete) | Delete program lines |
| [DIM](#dim) | Dimension arrays |
| [ELSE](#else) | IF...THEN...ELSE extension |
| [END](#end) | End program |
| [ENDIF](#endif) | End IF block |
| [ERL](#erl) | Last error line |
| [ERR](#err) | Last error code |
| [ERROR](#error) | Trigger error |
| [EXIT](#exit) | Exit interpreter |
| [FOR_NEXT](#for_next) | FOR...NEXT loop |
| [GET](#get) | Get record from file |

---

# Command Details

# AND

**BASIC Set:** Level II BASIC

## Syntax
```
expression AND expression
```

## Description
Performs a logical AND operation between two expressions.

## Parameters
- `expression`: Any valid expression

## Example
```
IF A > 0 AND B > 0 THEN PRINT "BOTH POSITIVE"
```

## Notes
- Used in IF and other logical expressions.

# APPEND

**BASIC Set:** Extension (used in file I/O)

## Syntax
```
OPEN "file" FOR APPEND AS #n
```

## Description
Specifies append mode when opening a file.

## Parameters
- `"file"`: Filename
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR APPEND AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only in file I/O statements.

# AS

**BASIC Set:** Extension (used in file I/O)

## Syntax
```
OPEN "file" FOR mode AS #n
```

## Description
Specifies the file number in file I/O operations.

## Parameters
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR INPUT AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only in file I/O statements.

# AT

**BASIC Set:** Level II BASIC

## Syntax
```
AT(n, m)
```

## Description
Specifies a screen position (row n, column m) for output.

## Parameters
- `n`: Row number
- `m`: Column number

## Example
```
PRINT AT(5,10); "Hello"
```

## Notes
- Used only in PRINT statements.

# BEEP

**BASIC Set:** Extension

## Syntax
```
BEEP
```

## Description
Produces a short beep sound.

## Parameters
None

## Example
```
BEEP
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- May not be supported on all systems.

# BYE

**BASIC Set:** Extension (alias)

## Syntax
```
BYE
```

## Description
Exits the BASIC interpreter (alias for EXIT and QUIT).

## Parameters
None

## Example
```
BYE
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Same as EXIT and QUIT.

# CASE

**BASIC Set:** Extension (BBC BASIC V compatible)

## Syntax
```
CASE expression OF
WHEN value1
  statements
WHEN value2
  statements
[OTHERWISE
  statements]
ENDCASE
```

## Description
Implements multi-way branching by comparing a single expression against multiple WHEN clauses. Each WHEN clause specifies a value to match; if the CASE expression equals that value, the associated statements execute. If provided, the OTHERWISE clause executes if no WHEN value matches.

## Parameters
- `expression`: Expression evaluated once and compared with each WHEN value
- `value`: Constant, variable, or expression to match against the CASE expression
- `statements`: One or more BASIC statements for each branch

## Example
```
10 INPUT CHOICE
20 CASE CHOICE OF
30 WHEN 1
40   PRINT "Option 1"
50 WHEN 2
60   PRINT "Option 2"
70 OTHERWISE
80   PRINT "Invalid"
90 ENDCASE
```

## Notes
- Must span multiple numbered lines (similar to IF-THEN-ENDIF blocks)
- No fall-through between WHEN clauses (unlike C switches)
- Each WHEN and the OTHERWISE clause require separate line numbers
- Can be nested and used within loops and subroutines
- The CASE expression is evaluated only once
- See: [CASE.md](CASE.md) for comprehensive documentation

# CIRCLE

**BASIC Set:** Extension

## Syntax
```
CIRCLE (x, y), r
```

## Description
Draws a circle at the specified coordinates with radius r.

## Parameters
- `x`: X coordinate
- `y`: Y coordinate
- `r`: Radius

## Example
```
CIRCLE (40, 12), 5
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for graphics operations.

# CLEAR

**BASIC Set:** Level II BASIC

## Syntax
```
CLEAR
```

## Description
Clears all variables and frees memory used by arrays.

## Parameters
None

## Example
```
CLEAR
```

## Notes
- Useful before running a new program.

# CLOSE

**BASIC Set:** Extension

## Syntax
```
CLOSE #n
```

## Description
Closes an open file.

## Parameters
- `#n`: File number to close

## Example
```
CLOSE #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- All files can be closed with `CLOSE` (no arguments).

# CLS

**BASIC Set:** Level II BASIC

## Syntax
```
CLS
```

## Description
Clears the screen and moves the cursor to the top left corner.

## Parameters
None

## Example
```
CLS
```

## Notes
- Useful for refreshing the display before output.

# COLOR

**BASIC Set:** Extension

## Syntax
```
COLOR n
```

## Description
Sets the foreground color for text or graphics.

## Parameters
- `n`: Color code

## Example
```
COLOR 2
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Color codes depend on system implementation.

# DATA / READ / RESTORE

**BASIC Set:** Level II BASIC

## Syntax
```
DATA value[, value...]
READ variable[, variable...]
RESTORE
```

## Description
DATA stores values in the program. READ reads them into variables. RESTORE resets the read pointer.

## Example
```
10 DATA 5, 10, 15
20 READ A, B, C
30 PRINT A, B, C
40 RESTORE
50 READ X, Y, Z
60 PRINT X, Y, Z
```

## Notes
- Use RESTORE to reread DATA from the start.

# DEFDBL

**BASIC Set:** Level II BASIC

## Syntax
```
DEFDBL letter-range [, ...]
```

## Description
Declares variables with names starting with specified letters as double-precision floats by default.

## Parameters
- `letter-range`: Range of letters (e.g., A-Z)

## Example
```
DEFDBL A-C, X, Z
```

## Notes
- Applies to all variables starting with those letters.

# DEFINT

**BASIC Set:** Level II BASIC

## Syntax
```
DEFINT letter-range [, ...]
```

## Description
Declares variables with names starting with specified letters as integers by default.

## Parameters
- `letter-range`: Range of letters (e.g., A-Z)

## Example
```
DEFINT A-C, X, Z
```

## Notes
- Applies to all variables starting with those letters.

# DEFSNG

**BASIC Set:** Level II BASIC

## Syntax
```
DEFSNG letter-range [, ...]
```

## Description
Declares variables with names starting with specified letters as single-precision floats by default.

## Parameters
- `letter-range`: Range of letters (e.g., A-Z)

## Example
```
DEFSNG A-C, X, Z
```

## Notes
- Applies to all variables starting with those letters.

# DEFSTR

**BASIC Set:** Level II BASIC

## Syntax
```
DEFSTR letter-range [, ...]
```

## Description
Declares variables with names starting with specified letters as strings by default.

## Parameters
- `letter-range`: Range of letters (e.g., A-Z)

## Example
```
DEFSTR A-C, X, Z
```

## Notes
- Applies to all variables starting with those letters.

# DELETE

**BASIC Set:** Extension

## Syntax
```
DELETE start[-end]
```

## Description
Deletes program lines in the specified range.

## Parameters
- `start[-end]`: Range of line numbers to delete

## Example
```
DELETE 10-20
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- If only one line is given, deletes that line.

# DIM

**BASIC Set:** Level II BASIC

## Syntax
```
DIM array(size1[, size2, ...])
```

## Description
Declares an array (or multi-dimensional array).

## Parameters
- `array`: Name of the array
- `size1, size2, ...`: Size of each dimension

## Example
```
10 DIM A(10)
20 FOR I = 1 TO 10
30 A(I) = I * I
40 NEXT I
50 PRINT A(5)
```

## Notes
- Arrays must be declared before use.

# ELSE

**BASIC Set:** Extension

## Syntax
```
IF condition THEN statement ELSE statement
```

## Description
Provides an alternative statement to execute if the IF condition is false.

## Parameters
- `condition`: Expression to evaluate
- `statement`: Statement to execute if true or false

## Example
```
IF A > 0 THEN PRINT "POS" ELSE PRINT "NEG"
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- ELSE is not supported in original Level II BASIC.

# END

**BASIC Set:** Level II BASIC

## Syntax
```
END
```

## Description
Stops the program.

## Example
```
10 PRINT "PROGRAM ENDS HERE"
20 END
```

## Notes
- Not strictly required; the program stops at the last line.

# ENDIF

**BASIC Set:** Extension

## Syntax
```
ENDIF
```

## Description
Marks the end of a multi-line IF block.

## Parameters
None

## Example
```
IF A > 0 THEN
  PRINT "POS"
ENDIF
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used in structured IF...ENDIF blocks.

# ERL

**BASIC Set:** Extension (system variable)

## Syntax
```
ERL
```

## Description
Returns the line number where the last error occurred.

## Parameters
None

## Example
```
PRINT ERL
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Value is set by the most recent error.

# ERR

**BASIC Set:** Extension (system variable)

## Syntax
```
ERR
```

## Description
Returns the error code of the last runtime error.

## Parameters
None

## Example
```
PRINT ERR
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Value is set by the most recent error.

# ERROR

**BASIC Set:** Extension

## Syntax
```
ERROR code
```

## Description
Triggers a runtime error with the specified error code, invoking the current error handler if set.

## Parameters
- `code`: Numeric error code to trigger

## Example
```
10 ERROR 53
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for custom error handling and testing.

# EXIT

**BASIC Set:** Extension

## Syntax
```
EXIT
```

## Description
Exits the BASIC interpreter and returns to the operating system.

## Parameters
None

## Example
```
EXIT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Same as BYE and QUIT.

# FOR...NEXT

**BASIC Set:** Level II BASIC

## Syntax
```
FOR variable = start TO end [STEP increment]
    ...
NEXT [variable]
```

## Description
Begins a counted loop. The variable is assigned the start value and incremented by the STEP value (default 1) until it exceeds the end value. The loop body executes for each value.

## Parameters
- `variable`: Numeric loop variable (auto-created if not defined)
- `start`: Initial value
- `end`: Final value
- `STEP increment` (optional): Amount to increment each time (default 1)

## Example
```
FOR I = 1 TO 5
  PRINT I
NEXT I
```

## Notes
- The loop variable is updated after each iteration.
- Exiting the loop early (e.g., with GOTO) is allowed but not recommended.
- Nested FOR...NEXT loops are supported.

# GET

**BASIC Set:** Extension

## Syntax
```
GET #n, variable
```

## Description
Reads a record from a file into a variable.

## Parameters
- `#n`: File number
- `variable`: Variable to store the data

## Example
```
GET #1, A$
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for random-access files.

