# RETURN

**BASIC Set:** Level II BASIC

## Syntax
```
RETURN
```

## Description
Returns program control to the statement following the most recent GOSUB.

## Parameters
None

## Example
```
100 PRINT "In subroutine"
110 RETURN
```

## Notes
- If RETURN is used without a matching GOSUB, a runtime error occurs.
