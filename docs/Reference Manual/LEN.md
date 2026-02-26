# LEN() Function

## Overview
Returns the number of characters in a string.

## Syntax
```basic
length = LEN(string)
```

## Parameters
- **string**: The string to measure

## Return Value
- Returns the number of characters in the string (integer)
- Returns 0 for empty strings

## Examples

### Basic Usage
```basic
10 A$ = "HELLO"
20 PRINT LEN(A$)  ' Output: 5
30 B$ = ""
40 PRINT LEN(B$)  ' Output: 0
```

### In Conditional Statements
```basic
10 INPUT "Enter a word: "; WORD$
20 IF LEN(WORD$) > 5 THEN
30   PRINT "Long word!"
40 ELSE
50   PRINT "Short word!"
60 END IF
```

### String Validation
```basic
10 INPUT "Enter password (min 6 chars): "; PWD$
20 IF LEN(PWD$) < 6 THEN
30   PRINT "Password too short!"
40   GOTO 10
50 END IF
```

## Notes
- Counts all characters including spaces
- Works with both regular strings and string variables
- Use in loops to validate string lengths

## Related Functions
- [MID$()](MID.md) - Extract substring
- [LEFT$()](LEFT.md) - Get leftmost characters
- [RIGHT$()](RIGHT.md) - Get rightmost characters
- [INSTR()](INSTR.md) - Find substring position
