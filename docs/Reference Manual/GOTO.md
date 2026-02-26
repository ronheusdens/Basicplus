# GOTO

**BASIC Set:** Level II BASIC

## Syntax
```
GOTO line_number
```

## Description
Unconditionally transfers program control to the specified line number.

## Parameters
- `line_number`: The line number to jump to

## Example
```
10 PRINT "Hello"
20 GOTO 10
```

## Notes
- If the line number does not exist, a runtime error occurs.
