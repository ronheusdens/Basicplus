# RIGHT$() Function

## Overview
Returns the rightmost (last) N characters from a string.

## Syntax
```basic
result$ = RIGHT$(string, n)
```

## Parameters
- **string**: The source string
- **n**: Number of characters to extract from the right (1-based)

## Return Value
- Returns a string containing the last N characters
- If N is greater than string length, returns the entire string
- If N ≤ 0, returns empty string

## Examples

### Basic Usage
```basic
10 A$ = "HELLO WORLD"
20 PRINT RIGHT$(A$, 5)  ' Output: WORLD
30 PRINT RIGHT$(A$, 1)  ' Output: D
40 PRINT RIGHT$(A$, 20) ' Output: HELLO WORLD
```

### Extract File Extension
```basic
10 FILENAME$ = "DOCUMENT.TXT"
20 EXT$ = RIGHT$(FILENAME$, 3)
30 PRINT "Extension: "; EXT$
```

### Extract Last Name
```basic
10 FULLNAME$ = "John Doe"
20 SPACE_POS = INSTR(FULLNAME$, " ")
30 LAST$ = RIGHT$(FULLNAME$, LEN(FULLNAME$) - SPACE_POS)
40 PRINT "Last name: "; LAST$
```

### Validate Filename Pattern
```basic
10 FILE$ = "mydata.csv"
20 IF RIGHT$(FILE$, 4) = ".csv" THEN
30   PRINT "Valid CSV file"
40 END IF
```

## Notes
- N must be non-negative
- If N is 0, returns empty string ""
- Safe to use with N larger than string length
- Useful for extracting suffixes or file extensions

## Related Functions
- [LEFT$()](LEFT.md) - Get leftmost characters
- [MID$()](MID.md) - Extract substring from middle
- [LEN()](LEN.md) - Get string length
