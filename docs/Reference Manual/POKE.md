# POKE

**BASIC Set:** Extension

## Syntax
```
POKE address, value
```

## Description
Stores a value at a specific memory address.

## Parameters
- `address`: Memory address
- `value`: Value to store (0-255)

## Example
```
POKE 16384, 255
```

## Notes
- Not part of Level II BASIC; provided as an extension.
- Use with caution; can crash the interpreter.
