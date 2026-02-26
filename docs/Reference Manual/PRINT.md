# PRINT

**BASIC Set:** Level II BASIC

## Syntax
```
PRINT [expression][,|; ...]
```

## Description
Displays the value of expressions on the screen. Commas and semicolons control spacing and formatting.

## Parameters
- `expression`: Value(s) to display
- `,` (comma): Tab to next zone
- `;` (semicolon): No space, concatenate output

## Example
```
10 PRINT "HELLO, WORLD!"
20 PRINT 1, 2, 3
```

## Notes
- Trailing semicolon suppresses newline.
- PRINT with no arguments outputs a blank line.
