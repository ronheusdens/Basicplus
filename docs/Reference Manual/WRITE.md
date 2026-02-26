# WRITE

**BASIC Set:** Extension

## Syntax
```
WRITE #n, expression [, ...]
```

## Description
Writes data to a file in a comma-separated format.

## Parameters
- `#n`: File number
- `expression`: Value(s) to write

## Example
```
WRITE #1, A, B, C
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Adds a newline after each WRITE.
