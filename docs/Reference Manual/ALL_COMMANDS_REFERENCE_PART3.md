# TRS-80 BASIC Command Reference (Q–Z)

This is Part 3 of the complete command reference. For quick navigation, see the Table of Contents below.

## Table of Contents

| Command | Description |
|---------|-------------|
| [QUIT](#quit) | Exit interpreter (alias) |
| [REM](#rem) | Comment |
| [RESET](#reset) | Clear pixel (graphics) |
| [RND](#rnd) | Generate random number |
| [RESUME](#resume) | Resume after error |
| [RETURN](#return) | Return from subroutine |
| [RUN](#run) | Run program |
| [SAVE](#save) | Save program to file |
| [SGN](#sgn) | Sign of number (-1, 0, or 1) |
| [SCREEN](#screen) | Set screen mode |
| [SET](#set) | Set pixel (graphics) |
| [SLEEP](#sleep) | Pause execution |
| [STEP](#step) | FOR loop increment |
| [STOP](#stop) | Stop program |
| [TAB](#tab) | Tab in output |
| [TO](#to) | FOR loop end value |
| [TROFF](#troff) | Turn off trace |
| [TRON](#tron) | Turn on trace |
| [USING](#using) | Formatted print |
| [WEND](#wend) | End WHILE loop |
| [WHILE](#while) | WHILE loop |
| [WRITE](#write) | Write to file |

---

# Command Details

# QUIT

**BASIC Set:** Extension (alias)

## Syntax
```
QUIT
```

## Description
Exits the BASIC interpreter (alias for EXIT and BYE).

## Parameters
None

## Example
```
QUIT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Same as EXIT and BYE.

# REM

**BASIC Set:** Level II BASIC

## Syntax
```
REM comment
```

## Description
Marks the rest of the line as a comment. The interpreter ignores everything after REM.

## Parameters
- `comment`: Any text (ignored by interpreter)

## Example
```
10 REM This is a comment
20 PRINT "Hello"
```

## Notes
- REM can be abbreviated as a single quote `'` in some dialects, but not in Level II BASIC.

# RESET

**BASIC Set:** Extension

## Syntax
```
RESET (x, y)
```

## Description
Clears a pixel at the specified coordinates.

## Parameters
- `x`: X coordinate
- `y`: Y coordinate

## Example
```
RESET (10, 20)
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for graphics operations.

# RESUME

**BASIC Set:** Extension

## Syntax
```
RESUME
RESUME n
RESUME NEXT
```

## Description
Resumes program execution after an error. `RESUME` returns to the line that caused the error, `RESUME n` jumps to line n, and `RESUME NEXT` continues with the next line.

## Parameters
- None, or:
- `n`: Line number to resume at
- `NEXT`: Continue after error line

## Example
```
100 PRINT "Error handler"
110 RESUME NEXT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only within an error handler.

# RETURN

**BASIC Set:** Level II BASIC

## Syntax
```
RETURN
```

## Description
Returns program control to the statement following the most recent GOSUB.

## Parameters
None

## Example
```
100 PRINT "In subroutine"
110 RETURN
```

## Notes
- If RETURN is used without a matching GOSUB, a runtime error occurs.

# RUN

**BASIC Set:** Level II BASIC

## Syntax
```
RUN
```

## Description
Starts execution of the current BASIC program from the first line or from a specified line number.

## Parameters
None

## Example
```
RUN
```

## Notes
- If a line number is specified (e.g., `RUN 100`), execution starts from that line.
- Any program currently in memory is executed.

# SAVE

**BASIC Set:** Extension

## Syntax
```
SAVE "file.bas"
```

## Description
Saves the current BASIC program to the specified file.

## Parameters
- `"file.bas"`: The filename to save to (must be in quotes)

## Example
```
SAVE "MYPROG.BAS"
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Overwrites the file if it already exists.

# SCREEN

**BASIC Set:** Extension

## Syntax
```
SCREEN n
```

## Description
Switches between text and graphics screen modes.

## Parameters
- `n`: Screen mode (0 = text, 1 = graphics)

## Example
```
SCREEN 1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Mode numbers may vary by implementation.

# SET

**BASIC Set:** Extension

## Syntax
```
SET (x, y)
```

## Description
Sets a pixel at the specified coordinates.

## Parameters
- `x`: X coordinate
- `y`: Y coordinate

## Example
```
SET (10, 20)
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for graphics operations.

# SLEEP

**BASIC Set:** Extension

## Syntax
```
SLEEP seconds
```

## Description
Pauses program execution for the specified number of seconds.

## Parameters
- `seconds`: Number of seconds to pause (can be fractional)

## Example
```
10 PRINT "Wait..."
20 SLEEP 2
30 PRINT "Done!"
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Useful for timing and delays.

# STEP

**BASIC Set:** Level II BASIC (used in FOR)

## Syntax
```
FOR variable = start TO end STEP increment
```

## Description
Specifies the increment value in a FOR loop.

## Parameters
- `increment`: Amount to add to the loop variable each time

## Example
```
FOR I = 1 TO 10 STEP 2
```

## Notes
- Used only in FOR...NEXT loops.

# STOP

**BASIC Set:** Level II BASIC

## Syntax
```
STOP
```

## Description
Halts program execution. Can be used for debugging or to end a program early.

## Parameters
None

## Example
```
10 PRINT "Before STOP"
20 STOP
30 PRINT "This will not run"
```

## Notes
- STOP is similar to END, but END is used to mark the logical end of a program.

# TAB

**BASIC Set:** Level II BASIC

## Syntax
```
TAB(n)
```

## Description
Moves the print position to column n.

## Parameters
- `n`: Column number

## Example
```
PRINT TAB(10); "Hello"
```

## Notes
- Used only in PRINT statements.

# TO

**BASIC Set:** Level II BASIC (used in FOR)

## Syntax
```
FOR variable = start TO end [STEP increment]
```

## Description
Specifies the end value in a FOR loop.

## Parameters
- `end`: Final value for the loop variable

## Example
```
FOR I = 1 TO 10
```

## Notes
- Used only in FOR...NEXT loops.

# TROFF

**BASIC Set:** Extension

## Syntax
```
TROFF
```

## Description
Disables trace mode.

## Parameters
None

## Example
```
TROFF
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Turns off line tracing enabled by TRON.

# TRON

**BASIC Set:** Extension

## Syntax
```
TRON
```

## Description
Enables trace mode, displaying each line as it is executed.

## Parameters
None

## Example
```
TRON
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Useful for debugging.

# USING

**BASIC Set:** Extension

## Syntax
```
PRINT USING format$; expression [, ...]
```

## Description
Prints values using a specified format string, similar to printf in C.

## Parameters
- `format$`: String specifying the output format
- `expression`: Value(s) to print

## Example
```
PRINT USING "###.##"; 12.345
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Useful for formatted numeric output.

# WEND

**BASIC Set:** Extension

## Syntax
```
WEND
```

## Description
Marks the end of a WHILE loop.

## Parameters
None

## Example
```
WHILE A < 10
  PRINT A
  A = A + 1
WEND
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Must be paired with WHILE.

# WHILE

**BASIC Set:** Extension

## Syntax
```
WHILE condition
  ...
WEND
```

## Description
Begins a loop that continues while the condition is true.

## Parameters
- `condition`: Expression to evaluate

## Example
```
WHILE A < 10
  PRINT A
  A = A + 1
WEND
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Must be paired with WEND.

# WRITE

**BASIC Set:** Extension

## Syntax
```
WRITE #n, expression [, ...]
```

## Description
Writes data to a file in a comma-separated format.

## Parameters
- `#n`: File number
- `expression`: Value(s) to write

## Example
```
WRITE #1, A, B, C
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Adds a newline after each WRITE.

