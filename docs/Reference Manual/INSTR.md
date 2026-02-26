# INSTR() Function

## Overview
Searches for a substring within a string and returns its position (1-based index), or 0 if not found.

## Syntax
```basic
pos = INSTR(string, substring)
pos = INSTR(start, string, substring)
```

## Parameters
- **string**: The string to search in
- **substring**: The substring to search for
- **start** (optional): Starting position for the search (1-based). If omitted, defaults to 1

## Return Value
- Returns the 1-based position of the first occurrence of `substring` in `string`
- Returns 0 if `substring` is not found or if `substring` is empty

## Examples

### Basic Search
```basic
10 A$ = "HELLO WORLD"
20 PRINT INSTR(A$, "WORLD")  ' Output: 7
30 PRINT INSTR(A$, "O")      ' Output: 5 (first O)
40 PRINT INSTR(A$, "XYZ")    ' Output: 0 (not found)
```

### Search with Start Position
```basic
10 S$ = "ABCABCABC"
20 PRINT INSTR(S$, "B")      ' Output: 2 (first occurrence)
30 PRINT INSTR(3, S$, "B")   ' Output: 5 (search from position 3)
40 PRINT INSTR(6, S$, "B")   ' Output: 8
```

### Find All Occurrences
```basic
10 S$ = "MISSISSIPPI"
20 POS = 0
30 WHILE 1
40   POS = INSTR(POS + 1, S$, "I")
50   IF POS = 0 THEN EXIT WHILE
60   PRINT "Found at position"; POS
70 WEND
```

## Notes
- INSTR is case-sensitive
- Position 0 or negative start position defaults to 1
- Empty substring returns 0
- If start position is beyond the string length, returns 0

## Related Functions
- [LEN()](LEN.md) - Get string length
- [MID$()](MID.md) - Extract substring
- [LEFT$()](LEFT.md) - Get leftmost characters
- [RIGHT$()](RIGHT.md) - Get rightmost characters
