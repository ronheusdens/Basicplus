## GOSUB

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
GOSUB line_number
```

**Description:**  
Calls a subroutine at the specified line number. Use RETURN to come back.

**Parameters:**
- `line_number`: The line to jump to

**Example:**
```basic
10 GOSUB 100
20 PRINT "Back from subroutine!"
30 END
100 PRINT "This is a subroutine."
110 RETURN
```

**Notes:**
- Use RETURN to return to the line after the GOSUB call.

---

## RETURN

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
RETURN
```

**Description:**  
Returns from a subroutine called with GOSUB.

**Example:**
```basic
10 GOSUB 100
20 PRINT "Back!"
30 END
100 PRINT "Subroutine."
110 RETURN
```

**Notes:**
- Always use RETURN to exit a subroutine.

---

## DIM

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
DIM array(size1[, size2, ...])
```

**Description:**  
Declares an array (or multi-dimensional array).

**Parameters:**
- `array`: Name of the array
- `size1, size2, ...`: Size of each dimension

**Example:**
```basic
10 DIM A(10)
20 FOR I = 1 TO 10
30 A(I) = I * I
40 NEXT I
50 PRINT A(5)
```

**Notes:**
- Arrays must be declared before use.

---

## DATA / READ / RESTORE

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
DATA value[, value...]
READ variable[, variable...]
RESTORE
```

**Description:**  
DATA stores values in the program. READ reads them into variables. RESTORE resets the read pointer.

**Example:**
```basic
10 DATA 5, 10, 15
20 READ A, B, C
30 PRINT A, B, C
40 RESTORE
50 READ X, Y, Z
60 PRINT X, Y, Z
```

**Notes:**
- Use RESTORE to reread DATA from the start.

---

## CLS

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
CLS
```

**Description:**  
Clears the screen.

**Example:**
```basic
10 PRINT "Hello"
20 CLS
30 PRINT "Screen cleared!"
```

**Notes:**
- Use at the start of graphics/text programs to clear the display.

---

## REM

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
REM [comment]
```

**Description:**  
Adds a comment to your program. Ignored by the interpreter.

**Example:**
```basic
10 REM This is a comment
20 PRINT "Hello"
```

**Notes:**
- Use comments to explain your code.

---

## STOP

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
STOP
```

**Description:**  
Stops the program, but you can continue with CONT.

**Example:**
```basic
10 PRINT "Before stop"
20 STOP
30 PRINT "After stop"
```

**Notes:**
- Use CONT to resume after STOP.

---

## SLEEP

**BASIC Set:**  
Extension

**Syntax:**
```basic
SLEEP n
```

**Description:**  
Pauses program execution for n seconds.

**Parameters:**
- `n`: Number of seconds to sleep

**Example:**
```basic
10 PRINT "Wait for 3 seconds..."
20 SLEEP 3
30 PRINT "Done!"
```

**Notes:**
- Not part of original Level II BASIC; added for convenience.
# TRS-80 Level II BASIC Command Reference

Welcome! This guide introduces the commands and statements supported by the TRS-80 Level II BASIC interpreter. Each command is explained with syntax, a plain-language description, and a working example.

---

## Table of Contents
- [PRINT](#print)
- [INPUT](#input)
- [IF...THEN](#ifthen)
- [GOTO](#goto)
- [FOR...NEXT](#fornext)
- [LET](#let)
- [END](#end)

---


## PRINT

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
PRINT [expression][, ...]
```

**Description:**  
Displays text or numbers on the screen.

**Parameters:**
- `expression`: The value(s) to print. Can be a string, number, or variable.

**Example:**
```basic
10 PRINT "HELLO, WORLD!"
20 PRINT 2 + 2
30 PRINT "THE ANSWER IS"; 42
```

**Notes:**
- Use `;` to print multiple items on the same line.
- Use `,` to print items in columns.

---


## INPUT

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
INPUT ["prompt";] variable
```

**Description:**  
Asks the user to enter a value.

**Parameters:**
- `prompt`: (Optional) Text to display before input
- `variable`: Where to store the user's input

**Example:**
```basic
10 INPUT "WHAT IS YOUR NAME"; N$
20 PRINT "HELLO, "; N$
```

**Notes:**
- String variables end with `$` (e.g., `N$`).

---


## IF...THEN

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
IF condition THEN statement
```

**Description:**  
Executes a statement if the condition is true.

**Parameters:**
- `condition`: A logical test (e.g., `A = 5`)
- `statement`: The command to run if true

**Example:**
```basic
10 INPUT "AGE"; A
20 IF A >= 18 THEN PRINT "ADULT"
30 IF A < 18 THEN PRINT "MINOR"
```

**Notes:**
- You can use `GOTO` or `GOSUB` after THEN for branching.

---


## GOTO

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
GOTO line_number
```

**Description:**  
Jumps to the specified line number in the program.

**Parameters:**
- `line_number`: The line to jump to

**Example:**
```basic
10 INPUT "SHOULD I LOOP?"; A$
20 IF A$ = "Y" THEN GOTO 10
30 PRINT "DONE!"
```

**Notes:**
- Use with care to avoid infinite loops.

---


## FOR...NEXT

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
FOR variable = start TO end [STEP step]
  ...
NEXT [variable]
```

**Description:**  
Repeats a block of code a set number of times.

**Parameters:**
- `variable`: Loop counter
- `start`: Starting value
- `end`: Ending value
- `step`: (Optional) Amount to increment each time

**Example:**
```basic
10 FOR I = 1 TO 5
20 PRINT "COUNT: ", I
30 NEXT I
```

**Notes:**
- STEP defaults to 1 if omitted.

---


## LET

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
LET variable = expression
```

**Description:**  
Assigns a value to a variable.

**Parameters:**
- `variable`: The variable to assign
- `expression`: The value or calculation

**Example:**
```basic
10 LET X = 5 * 2
20 PRINT X
```

**Notes:**
- `LET` is optional; you can write `X = 5` instead of `LET X = 5`.

---


## END

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
END
```

**Description:**  
Stops the program.

**Example:**
```basic
10 PRINT "PROGRAM ENDS HERE"
20 END
```

**Notes:**
- Not strictly required; the program stops at the last line.

---

Add more commands using this structure for a complete reference!
