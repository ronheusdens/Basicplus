# STR$() Function

## Overview
Converts a number to its string representation.

## Syntax
```basic
string$ = STR$(number)
```

## Parameters
- **number**: Numeric value to convert

## Return Value
- String representation of the number

## Examples
```basic
10 PRINT STR$(123)     ' Output: 123
20 PRINT STR$(-5.5)    ' Output: -5.5
30 PRINT STR$(0)       ' Output: 0
```

## Notes
- Output is not padded or formatted
- Useful for string concatenation and output

## Related Functions
- [VAL()](VAL.md) - Convert string to number
