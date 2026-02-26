# RESUME

**BASIC Set:** Extension

## Syntax
```
RESUME
RESUME n
RESUME NEXT
```

## Description
Resumes program execution after an error. `RESUME` returns to the line that caused the error, `RESUME n` jumps to line n, and `RESUME NEXT` continues with the next line.

## Parameters
- None, or:
- `n`: Line number to resume at
- `NEXT`: Continue after error line

## Example
```
100 PRINT "Error handler"
110 RESUME NEXT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only within an error handler.
