# OPEN

**BASIC Set:** Extension

## Syntax
```
OPEN "file" FOR mode AS #n
```

## Description
Opens a file for input, output, or append operations.

## Parameters
- `"file"`: Filename
- `mode`: INPUT, OUTPUT, or APPEND
- `#n`: File number

## Example
```
OPEN "DATA.TXT" FOR INPUT AS #1
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- File must be closed with CLOSE.
