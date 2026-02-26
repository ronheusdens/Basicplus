# GOSUB

**BASIC Set:** Level II BASIC

## Syntax
```
GOSUB line_number
```

## Description
Transfers program control to the specified line number, saving the return address. Use RETURN to resume execution after the subroutine.

## Parameters
- `line_number`: The line number to jump to

## Example
```
10 GOSUB 100
20 PRINT "Back from subroutine"
30 END
100 PRINT "In subroutine"
110 RETURN
```

## Notes
- GOSUB/RETURN can be nested.
- If RETURN is missing, a runtime error occurs.
