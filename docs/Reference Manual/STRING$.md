# STRING$() Function

## Overview
Returns a string consisting of a repeated character.

## Syntax
```basic
result$ = STRING$(count, char)
result$ = STRING$(count, code)
```

## Parameters
- **count**: Number of times to repeat (0–255)
- **char**: Single-character string or ASCII code

## Return Value
- String of length count, filled with the character
- Returns empty string if count ≤ 0

## Examples
```basic
10 PRINT STRING$(5, "*")    ' Output: *****
20 PRINT STRING$(3, 65)      ' Output: AAA
30 PRINT STRING$(0, "X")     ' Output: (empty string)
```

## Notes
- If char is a string, only the first character is used
- If char is a code, must be 0–255
- Useful for padding and formatting

## Related Functions
- [CHR$()](CHR$.md) - Character from code
