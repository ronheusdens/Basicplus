# CHR$() Function

## Overview
Returns a one-character string for the given ASCII code.

## Syntax
```basic
char$ = CHR$(code)
```

## Parameters
- **code**: Integer ASCII code (0–255)

## Return Value
- Single-character string for the code
- Returns empty string if code is out of range

## Examples
```basic
10 PRINT CHR$(65)      ' Output: A
20 PRINT CHR$(13)      ' Output: (carriage return)
30 PRINT CHR$(256)     ' Output: (empty string)
```

## Notes
- Only codes 0–255 are valid
- Useful for control characters and binary data

## Related Functions
- [ASC()](ASC.md) - ASCII code of character
