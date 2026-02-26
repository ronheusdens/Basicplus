# TRS-80 BASIC Command Reference (G–P)

This is Part 2 of the complete command reference. For quick navigation, see the Table of Contents below.

## Table of Contents

| Command | Description |
|---------|-------------|
| [GOSUB](#gosub) | Subroutine call |
| [GOTO](#goto) | Unconditional jump |
| [IF_THEN](#if_then) | IF...THEN statement |
| [INT](#int) | Integer part of number |
| [INSTR](#instr) | Find substring position |
| [INPUT](#input) | User input |
| [LET](#let) | Assignment |
| [LINE](#line) | Read line from file |
| [LIST](#list) | List program lines |
| [LOAD](#load) | Load program from file |
| [MOD](#mod) | Modulo operation |
| [NEW](#new) | Clear program |
| [NEXT](#next) | End FOR loop |
| [NOT](#not) | Logical NOT |
| [ON](#on) | ON...GOTO/GOSUB |
| [ON_ERROR_GOTO](#on_error_goto) | Set error handler |
| [OPEN](#open) | Open a file |
| [OR](#or) | Logical OR |
| [OUTPUT](#output) | Output file mode |
| [PAINT](#paint) | Fill area (graphics) |
| [PCOLOR](#pcolor) | Set background color |
| [POKE](#poke) | Write to memory |
| [PRINT](#print) | Print output |
| [PUT](#put) | Write record to file |

---

# Command Details

# GOSUB

**BASIC Set:** Level II BASIC

## Syntax
```
GOSUB line_number
```

## Description
Transfers program control to the specified line number, saving the return address. Use RETURN to resume execution after the subroutine.

## Parameters
- `line_number`: The line number to jump to

## Example
```
10 GOSUB 100
20 PRINT "Back from subroutine"
30 END
100 PRINT "In subroutine"
110 RETURN
```

## Notes
- GOSUB/RETURN can be nested.
- If RETURN is missing, a runtime error occurs.

# GOTO

**BASIC Set:** Level II BASIC

## Syntax
```
GOTO line_number
```

## Description
Unconditionally transfers program control to the specified line number.

## Parameters
- `line_number`: The line number to jump to

## Example
```
10 PRINT "Hello"
20 GOTO 10
```

## Notes
- If the line number does not exist, a runtime error occurs.

# IF...THEN

**BASIC Set:** Level II BASIC

## Syntax
```
IF condition THEN statement
```

## Description
Executes the statement if the condition is true. The statement can be a line number (for GOTO) or any valid BASIC statement.

## Parameters
- `condition`: An expression that evaluates to true or false
- `statement`: Statement or line number to execute if condition is true

## Example
```
10 INPUT A
20 IF A > 0 THEN PRINT "Positive"
```

## Notes
- Only a single statement is allowed after THEN.
- ELSE is not supported in Level II BASIC.
# INSTR

**BASIC Set:** Level II BASIC

## Syntax
```
position = INSTR(string, substring)
position = INSTR(start, string, substring)
```

## Description
Searches for a substring within a string and returns its 1-based position, or 0 if not found.

## Parameters
- `string`: The string to search in
- `substring`: The substring to search for
- `start` (optional): Starting position for the search (1-based). If omitted, defaults to 1

## Example
```
10 A$ = "HELLO WORLD"
20 PRINT INSTR(A$, "WORLD")    ' Output: 7
30 PRINT INSTR(A$, "O")        ' Output: 5
40 PRINT INSTR(A$, "XYZ")      ' Output: 0
50 PRINT INSTR(3, A$, "O")     ' Output: 5 (search from position 3)
```

## Notes
- INSTR is case-sensitive
- Empty substring returns 0
- If start position is beyond the string length, returns 0
- Returns position of first occurrence only
# INPUT

**BASIC Set:** Level II BASIC

## Syntax
```
INPUT ["prompt";] variable [, variable ...]
```

## Description
Prompts the user for input and assigns the entered value(s) to the specified variable(s).

## Parameters
- `"prompt"` (optional): Text to display before input
- `variable`: Variable(s) to assign input values

## Example
```
10 INPUT "Enter a number"; N
20 PRINT N
```

## Notes
- Multiple variables can be read in one statement.
- String and numeric variables are supported.

# LET

**BASIC Set:** Level II BASIC

## Syntax
```
LET variable = expression
```

## Description
Assigns the value of an expression to a variable. The LET keyword is optional in most cases.

## Parameters
- `variable`: The variable to assign
- `expression`: The value or calculation to assign

## Example
```
10 LET X = 5 * 2
20 PRINT X
```

## Notes
- `LET` is optional: `X = 5` is equivalent.

# LINE

**BASIC Set:** Extension

## Syntax
```
LINE INPUT #n, variable
```

## Description
Reads a line of text from a file into a variable.

## Parameters
- `#n`: File number
- `variable`: Variable to store the line

## Example
```
LINE INPUT #1, A$
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Reads up to a newline character.

# LIST

**BASIC Set:** Level II BASIC

## Syntax
```
LIST [start[-end]]
```

## Description
Displays all or part of the current BASIC program, showing each line with its line number.

## Parameters
- `start[-end]` (optional): Range of line numbers to list

## Example
```
LIST
LIST 10-50
```

## Notes
- If no range is given, the entire program is listed.

# LOAD

**BASIC Set:** Extension

## Syntax
```
LOAD "file.bas"
```

## Description
Loads a BASIC program from the specified file into memory, replacing the current program.

## Parameters
- `"file.bas"`: The filename to load (must be in quotes)

## Example
```
LOAD "MYPROG.BAS"
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- The current program is replaced.

# MOD

**BASIC Set:** Extension

## Syntax
```
expression MOD expression
```

## Description
Returns the remainder after division of two numbers.

## Parameters
- `expression`: Any valid numeric expression

## Example
```
PRINT 10 MOD 3
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used in arithmetic expressions.

# NEW

**BASIC Set:** Level II BASIC

## Syntax
```
NEW
```

## Description
Clears the current BASIC program from memory, allowing you to start a new one.

## Parameters
None

## Example
```
NEW
```

## Notes
- All program lines and variables are erased.

# NEXT

**BASIC Set:** Level II BASIC

## Syntax
```
NEXT [variable]
```

## Description
Marks the end of a FOR loop and increments the loop variable.

## Parameters
- `variable` (optional): The loop variable

## Example
```
FOR I = 1 TO 5
  PRINT I
NEXT I
```

## Notes
- If variable is omitted, the innermost FOR loop is closed.

# NOT

**BASIC Set:** Level II BASIC

## Syntax
```
NOT expression
```

## Description
Performs a logical NOT operation, inverting the value of the expression.

## Parameters
- `expression`: Any valid expression

## Example
```
IF NOT A THEN PRINT "A is zero"
```

## Notes
- Used in IF and other logical expressions.

# ON

**BASIC Set:** Level II BASIC

## Syntax
```
ON expression GOTO line1, line2, ...
ON expression GOSUB line1, line2, ...
```

## Description
Transfers control to one of several lines, depending on the value of the expression.

## Parameters
- `expression`: Determines which line to jump to
- `line1, line2, ...`: List of line numbers

## Example
```
ON X GOTO 100, 200, 300
```

## Notes
- If the expression is out of range, no jump occurs.

# ON ERROR GOTO

**BASIC Set:** Extension

## Syntax
```
ON ERROR GOTO n
ON ERROR GOTO 0
```

## Description
Sets or disables an error handler. When enabled, program control jumps to line `n` if a runtime error occurs. Using `0` disables the error handler.

## Parameters
- `n`: Line number to handle errors
- `0`: Disables error handler

## Example
```
10 ON ERROR GOTO 100
20 PRINT 1/0
30 END
100 PRINT "Error occurred!"
110 RESUME NEXT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Only one error handler can be active at a time.

# OPEN

**BASIC Set:** Extension

## Syntax
```
OPEN "file" FOR mode AS #n
```

## Description
Opens a file for input, output, or append operations.

## Parameters
- `"file"`: Filename
- `mode`: INPUT, OUTPUT, or APPEND
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR INPUT AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- File must be closed with CLOSE.

# OR

**BASIC Set:** Level II BASIC

## Syntax
```
expression OR expression
```

## Description
Performs a logical OR operation between two expressions.

## Parameters
- `expression`: Any valid expression

## Example
```
IF A = 0 OR B = 0 THEN PRINT "ZERO"
```

## Notes
- Used in IF and other logical expressions.

# OUTPUT

**BASIC Set:** Extension (used in file I/O)

## Syntax
```
OPEN "file" FOR OUTPUT AS #n
```

## Description
Specifies output mode when opening a file.

## Parameters
- `"file"`: Filename
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR OUTPUT AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only in file I/O statements.

# PAINT

**BASIC Set:** Extension

## Syntax
```
PAINT (x, y)
```

## Description
Fills an area starting at the specified coordinates.

## Parameters
- `x`: X coordinate
- `y`: Y coordinate

## Example
```
PAINT (10, 20)
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for graphics operations.

# PCOLOR

**BASIC Set:** Extension

## Syntax
```
PCOLOR n
```

## Description
Sets the background color for text or graphics.

## Parameters
- `n`: Color code

## Example
```
PCOLOR 4
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Color codes depend on system implementation.

# POKE

**BASIC Set:** Extension

## Syntax
```
POKE address, value
```

## Description
Stores a value at a specific memory address.

## Parameters
- `address`: Memory address
- `value`: Value to store (0-255)

## Example
```
POKE 16384, 255
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Use with caution; can crash the interpreter.

# PRINT

**BASIC Set:** Level II BASIC

## Syntax
```
PRINT [expression][,|; ...]
```

## Description
Displays the value of expressions on the screen. Commas and semicolons control spacing and formatting.

## Parameters
- `expression`: Value(s) to display
- `,` (comma): Tab to next zone
- `;` (semicolon): No space, concatenate output

## Example
```
10 PRINT "HELLO, WORLD!"
20 PRINT 1, 2, 3
```

## Notes
- Trailing semicolon suppresses newline.
- PRINT with no arguments outputs a blank line.

# PUT

**BASIC Set:** Extension

## Syntax
```
PUT #n, variable
```

## Description
Writes a record from a variable to a file.

## Parameters
- `#n`: File number
- `variable`: Variable containing the data

## Example
```
PUT #1, A$
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used for random-access files.

