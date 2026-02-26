# CASE Statement Reference

## Overview

The **CASE** statement implements multi-way branching by comparing a single expression against multiple **WHEN** clauses. It provides a clean alternative to nested [IF-THEN-ELSE](IF_THEN.md) structures for selecting among many options.

CASE statements in this implementation are converted internally to nested IF-THEN-ELSE chains at parse time, ensuring compatibility with the existing executor and maintaining full support for complex nested structures.

## Syntax

```basic
line_num CASE expression OF
line_num WHEN value1
line_num   statements
line_num WHEN value2
line_num   statements
line_num [OTHERWISE
line_num   statements]
line_num ENDCASE
```

### Components

- **CASE** – Keyword initiating the case statement
- **expression** – Expression to evaluate (compared against each WHEN value)
- **OF** – Keyword separating expression from WHEN clauses
- **WHEN** – Keyword introducing a case branch; followed by a value to match
- **value** – Constant, variable, or expression compared with the CASE expression
- **statements** – One or more BASIC statements (can span multiple lines)
- **OTHERWISE** – Optional default branch executed if no WHEN matches
- **ENDCASE** – Keyword terminating the CASE block

## Rules

1. **Multi-line Structure**: CASE blocks must span multiple numbered lines (similar to [IF-THEN-ENDIF](IF_THEN.md) blocks)
2. **Line Numbers**: Each segment (CASE, WHEN, OTHERWISE, ENDCASE) must have its own line number
3. **Expression Evaluation**: The CASE expression is evaluated once and compared with each WHEN value
4. **Single Match**: Only the first matching WHEN branch executes; remaining branches are skipped
5. **No Fall-Through**: Unlike C switches, execution does not "fall through" to subsequent WHEN clauses
6. **OTHERWISE Default**: If provided, OTHERWISE executes when no WHEN value matches
7. **Nesting**: CASE statements can be nested within other control structures and vice versa

## Examples

### Simple Numeric Comparison

```basic
10 REM Select action by number
20 PRINT "Enter choice (1-3): "
30 INPUT X
40 CASE X OF
50 WHEN 1
60   PRINT "You selected ONE"
70 WHEN 2
80   PRINT "You selected TWO"
90 WHEN 3
100  PRINT "You selected THREE"
110 OTHERWISE
120  PRINT "Invalid choice"
130 ENDCASE
140 END
```

### String-Based Branching

```basic
10 REM Process command
20 INPUT "Command: ", CMD$
30 CASE CMD$ OF
40 WHEN "QUIT"
50   PRINT "Exiting..."
60   END
70 WHEN "HELP"
80   PRINT "Available commands: QUIT, HELP, INFO"
90 WHEN "INFO"
100  PRINT "This is a demo program"
110 OTHERWISE
120  PRINT "Unknown command"
130 ENDCASE
140 GOTO 20
```

### Expression Matching

```basic
10 REM Process based on expression
20 INPUT A, B
30 CASE A + B OF
40 WHEN 0
50   PRINT "Sum is zero"
60 WHEN 10
70   PRINT "Sum is ten"
80 WHEN 20
90   PRINT "Sum is twenty"
100 OTHERWISE
110  PRINT "Other sum: "; A + B
120 ENDCASE
```

### CASE Within a Loop

```basic
10 REM Process items in a loop
20 FOR I = 1 TO 5
30   CASE I OF
40   WHEN 1
50     PRINT "First item"
60   WHEN 5
70     PRINT "Last item"
80   OTHERWISE
90     PRINT "Item "; I
100  ENDCASE
110 NEXT I
120 END
```

### CASE Within a Subroutine

```basic
10 REM Menu handler
20 PRINT "1=Add, 2=Remove, 3=List: "
30 INPUT CHOICE
40 GOSUB 100
50 END

100 REM Process menu choice
110 CASE CHOICE OF
120 WHEN 1
130   PRINT "Adding item..."
140 WHEN 2
150   PRINT "Removing item..."
160 WHEN 3
170   PRINT "Listing items..."
180 OTHERWISE
190   PRINT "Invalid choice"
200 ENDCASE
210 RETURN
```

### Nested CASE Statements

```basic
10 REM Nested CASE for game logic
20 INPUT "Level (1-3): ", L
30 CASE L OF
40 WHEN 1
50   PRINT "Easy mode"
60   CASE RND(1)*3+1 OF
70   WHEN 1
80     PRINT "Enemy: Goblin"
90   WHEN 2
100    PRINT "Enemy: Orc"
110   WHEN 3
120    PRINT "Enemy: Troll"
130   ENDCASE
140 WHEN 2
150   PRINT "Medium mode"
160 WHEN 3
170   PRINT "Hard mode"
180 ENDCASE
190 END
```

## Implementation Details

### Internal Conversion

CASE statements are converted to nested IF-THEN-ELSE chains at parse time:

```basic
CASE X OF
WHEN 1
   statement1
WHEN 2
   statement2
OTHERWISE
   statement3
ENDCASE
```

Is internally converted to:

```basic
IF X = 1 THEN
   statement1
ELSE
IF X = 2 THEN
   statement2
ELSE
   statement3
ENDIF
ENDIF
```

### Expression Copying

The CASE expression is deep-copied for each WHEN comparison to ensure independent evaluation and prevent side effects from affecting multiple branches.

## Common Patterns

### Type-Based Dispatch

```basic
100 CASE TYPE$ OF
110 WHEN "NPC"
120   GOSUB 500: REM Handle NPC
130 WHEN "ITEM"
140   GOSUB 600: REM Handle item
150 WHEN "TRAP"
160   GOSUB 700: REM Handle trap
170 ENDCASE
```

### State Machine

```basic
100 CASE STATE OF
110 WHEN 0
120   PRINT "Init"
130   STATE = 1
140 WHEN 1
150   PRINT "Running"
160   IF condition THEN STATE = 2
170 WHEN 2
180   PRINT "Done"
190 ENDCASE
```

### Range Simulation with Multiple WHEN Values

```basic
100 CASE SCORE OF
110 WHEN 90
120   PRINT "A"
130 WHEN 80
140   PRINT "B"
150 WHEN 70
160   PRINT "C"
170 OTHERWISE
180   PRINT "F"
190 ENDCASE
```

## Limitations

1. **No Fall-Through**: Execution does immediately terminates after a matching WHEN (unlike C)
2. **No Range Matching**: Each WHEN compares a single value; use multiple CASE statements for range checks
3. **Single Expression**: Only one expression is evaluated; complex multi-condition logic requires nested CASE or IF

## Error Handling

- **Parse Error: Expected ENDCASE** – Missing ENDCASE terminator
- **Parse Error: Expected OF** – Missing OF keyword
- **Parse Error: Expected value after WHEN** – WHEN clause has no comparison value

## Related Commands

- [IF-THEN-ENDIF](IF_THEN.md) – General conditional branching
- [ON-GOTO](ON.md) – Computed GOTO branching
- [WHILE-WEND](WHILE.md) – Loop with condition testing
- [DO-LOOP](DO_LOOP.md) – Loop with exit conditions

## Notes

- CASE provides cleaner syntax than deeply nested IF statements
- All WHEN and OTHERWISE branches must start on separate numbered lines
- The CASE expression is evaluated only once (efficient for complex expressions)
- Can be nested arbitrarily deep within other control structures
- Works with numeric values, strings, and expressions

## Example Program: Grade Converter

```basic
10 REM Grade converter using CASE
20 PRINT "Enter score (0-100): "
30 INPUT SCORE
40 CASE INT(SCORE/10) OF
50 WHEN 10
60   PRINT "A+ "
70 WHEN 9
80   PRINT "A"
90 WHEN 8
100  PRINT "B"
110 WHEN 7
120  PRINT "C"
130 WHEN 6
140  PRINT "D"
150 OTHERWISE
160  PRINT "F"
170 ENDCASE
180 END
```

Output:
```
Enter score (0-100): 85
B
```

---

**Last Updated:** February 2026  
**Status:** Fully Implemented
