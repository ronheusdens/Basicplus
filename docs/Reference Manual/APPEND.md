# APPEND

**BASIC Set:** Extension (used in file I/O)

## Syntax
```
OPEN "file" FOR APPEND AS #n
```

## Description
Specifies append mode when opening a file.

## Parameters
- `"file"`: Filename
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR APPEND AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Used only in file I/O statements.
