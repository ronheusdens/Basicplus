# MID$() Function

## Overview
Returns a substring from the middle of a string, starting at a specified position.

## Syntax
```basic
result$ = MID$(string, start)
result$ = MID$(string, start, length)
```

## Parameters
- **string**: The source string
- **start**: Starting position (1-based index)
- **length** (optional): Number of characters to extract. If omitted, extracts to end of string

## Return Value
- Returns a substring starting at the specified position
- If start is beyond string length, returns empty string ""
- If length extends beyond string end, returns only available characters

## Examples

### Extract from Position
```basic
10 A$ = "HELLO WORLD"
20 PRINT MID$(A$, 7)       ' Output: WORLD
30 PRINT MID$(A$, 7, 5)    ' Output: WORLD
40 PRINT MID$(A$, 7, 3)    ' Output: WOR
```

### Extract Every Other Character
```basic
10 S$ = "123456789"
20 FOR I = 1 TO LEN(S$) STEP 2
30   PRINT MID$(S$, I, 1);
40 NEXT I
50 PRINT  ' Output: 13579
```

### Parse Fixed-Format Data
```basic
10 RECORD$ = "12341500150Fred    "
20 ID = VAL(MID$(RECORD$, 1, 4))      ' ID: 1234
30 PRICE = VAL(MID$(RECORD$, 5, 4))   ' Price: 1500
40 QTY = VAL(MID$(RECORD$, 9, 3))     ' Qty: 150
50 NAME$ = MID$(RECORD$, 12, 8)       ' Name: Fred
```

### Extract Words from String
```basic
10 S$ = "The Quick Brown Fox"
20 SPACE_POS = INSTR(S$, " ")
30 WORD1$ = LEFT$(S$, SPACE_POS - 1)
40 REST$ = MID$(S$, SPACE_POS + 1)
50 PRINT "First: "; WORD1$; ", Rest: "; REST$
```

## Notes
- Position is 1-based (first character is at position 1)
- If start ≤ 0, treated as position 1
- If length is omitted, extracts from start to end of string
- If start is beyond string length, returns empty string
- Useful for parsing formatted strings and data records

## Related Functions
- [LEFT$()](LEFT.md) - Get leftmost characters
- [RIGHT$()](RIGHT.md) - Get rightmost characters
- [LEN()](LEN.md) - Get string length
- [INSTR()](INSTR.md) - Find substring position
