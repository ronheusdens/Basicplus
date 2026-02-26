# ASC() Function

## Overview
Returns the ASCII code of the first character in a string.

## Syntax
```basic
code = ASC(string)
```

## Parameters
- **string**: The string to examine

## Return Value
- ASCII code (0–255) of the first character
- Returns 0 if the string is empty

## Examples
```basic
10 PRINT ASC("A")      ' Output: 65
20 PRINT ASC("Hello")  ' Output: 72
30 PRINT ASC("")       ' Output: 0
```

## Notes
- Only the first character is used
- For non-ASCII characters, result may vary

## Related Functions
- [CHR$()](CHR$.md) - Character from code
