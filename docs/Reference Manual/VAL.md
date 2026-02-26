# VAL() Function

## Overview
Converts a string to a numeric value.

## Syntax
```basic
number = VAL(string)
```

## Parameters
- **string**: String to convert

## Return Value
- Numeric value represented by the string
- Returns 0 if the string does not start with a number

## Examples
```basic
10 PRINT VAL("123")    ' Output: 123
20 PRINT VAL("-5.5")   ' Output: -5.5
30 PRINT VAL("abc")    ' Output: 0
```

## Notes
- Stops parsing at first non-numeric character
- Ignores leading spaces
- Useful for input validation and conversion

## Related Functions
- [STR$()](STR$.md) - Convert number to string
