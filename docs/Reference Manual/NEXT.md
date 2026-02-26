# NEXT

**BASIC Set:** Level II BASIC

## Syntax
```
NEXT [variable]
```

## Description
Marks the end of a FOR loop and increments the loop variable.

## Parameters
- `variable` (optional): The loop variable

## Example
```
FOR I = 1 TO 5
  PRINT I
NEXT I
```

## Notes
- If variable is omitted, the innermost FOR loop is closed.
