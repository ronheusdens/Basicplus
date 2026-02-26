# ON ERROR GOTO

**BASIC Set:** Extension

## Syntax
```
ON ERROR GOTO n
ON ERROR GOTO 0
```

## Description
Sets or disables an error handler. When enabled, program control jumps to line `n` if a runtime error occurs. Using `0` disables the error handler.

## Parameters
- `n`: Line number to handle errors
- `0`: Disables error handler

## Example
```
10 ON ERROR GOTO 100
20 PRINT 1/0
30 END
100 PRINT "Error occurred!"
110 RESUME NEXT
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Only one error handler can be active at a time.
