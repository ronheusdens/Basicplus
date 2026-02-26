# ON

**BASIC Set:** Level II BASIC

## Syntax
```
ON expression GOTO line1, line2, ...
ON expression GOSUB line1, line2, ...
```

## Description
Transfers control to one of several lines, depending on the value of the expression.

## Parameters
- `expression`: Determines which line to jump to
- `line1, line2, ...`: List of line numbers

## Example
```
ON X GOTO 100, 200, 300
```

## Notes
- If the expression is out of range, no jump occurs.
