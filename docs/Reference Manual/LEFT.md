# LEFT$() Function

## Overview
Returns the leftmost (first) N characters from a string.

## Syntax
```basic
result$ = LEFT$(string, n)
```

## Parameters
- **string**: The source string
- **n**: Number of characters to extract from the left (1-based)

## Return Value
- Returns a string containing the first N characters
- If N is greater than string length, returns the entire string
- If N ≤ 0, returns empty string

## Examples

### Basic Usage
```basic
10 A$ = "HELLO WORLD"
20 PRINT LEFT$(A$, 5)  ' Output: HELLO
30 PRINT LEFT$(A$, 1)  ' Output: H
40 PRINT LEFT$(A$, 20) ' Output: HELLO WORLD
```

### Extract First Name
```basic
10 FULLNAME$ = "John Doe"
20 FIRST$ = LEFT$(FULLNAME$, INSTR(FULLNAME$, " ") - 1)
30 PRINT "First name: "; FIRST$
```

### Truncate Long Text
```basic
10 TITLE$ = "This is a very long title"
20 SHORT$ = LEFT$(TITLE$, 10)
30 PRINT SHORT$ ' Output: This is a 
```

## Notes
- N must be non-negative
- If N is 0, returns empty string ""
- Safe to use with N larger than string length
- Useful for extracting prefixes or field values

## Related Functions
- [RIGHT$()](RIGHT.md) - Get rightmost characters
- [MID$()](MID.md) - Extract substring from middle
- [LEN()](LEN.md) - Get string length
