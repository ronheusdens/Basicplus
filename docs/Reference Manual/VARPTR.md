# VARPTR Function

## Overview

**VARPTR** is a TRS-80 BASIC function that returns the memory address of a variable. It's used for low-level programming when you need to reference variable locations, typically in conjunction with the `USR` function for machine code routines or other advanced memory operations.

## Syntax

```basic
address = VARPTR(variable_name)
```

## Parameters

- **variable_name**: The name of a variable (numeric or string). Must be passed as a bare variable identifier, not a string literal.

## Return Value

Returns a numeric address value representing the simulated location of the variable in BASIC memory space.

## Description

VARPTR computes a hash-based address for the given variable name and returns a value in the upper half of the BASIC memory space (addresses 16384-32767 by default, depending on memory size).

The implementation:
1. Takes the variable name as input
2. Computes a 31-based hash of the variable characters
3. Returns: `(memory_size / 2 + hash % (memory_size / 2))`

This provides unique, repeatable addresses for variables while simulating the TRS-80 variable storage layout.

## Examples

### Basic Usage

```basic
A = 42
ADDR = VARPTR(A)
PRINT "Variable A is at address:"; ADDR
```

Output:
```
Variable A is at address: 16425
```

### Multiple Variables

```basic
X = 100
Y = 200
NAME$ = "HELLO"

PRINT VARPTR(X)
PRINT VARPTR(Y)
PRINT VARPTR(NAME$)
```

Each variable gets a unique address based on its name.

### With USR Function

```basic
10 A = 50
20 ADDR = VARPTR(A)
30 RESULT = USR(1000, ADDR)
40 PRINT RESULT
```

This pattern is used when machine code routines need to access BASIC variables by address.

## Notes

- **Hash-based addresses**: The returned address is a simulated address based on a hash of the variable name, not an actual RAM pointer
- **Deterministic**: The same variable always returns the same VARPTR value
- **Unique per variable**: Different variable names produce different VARPTR addresses
- **Case-sensitive**: `VAR` and `var` are treated as different variable names
- **All variable types supported**: Works with numeric variables (integers, floats) and string variables (with `$` suffix)
- **No arguments**: VARPTR requires exactly one argument—the variable name itself

## Limitations

- Cannot use expressions inside VARPTR: `VARPTR(X+1)` is invalid; use `VARPTR(X)` instead
- Cannot pass variable names as strings: `VARPTR("X")` is invalid; use `VARPTR(X)`
- Returned addresses are simulated, not actual RAM locations in memory

## Platform Support

- **macOS**: Full support
- **Linux**: Full support
- **Windows**: Full support

## Related Functions

- **USR(address)** - Calls machine code routines at specified addresses
- **PEEK(address)** - Reads a byte from memory
- **POKE(address, value)** - Writes a byte to memory
- **FRE(expr)** - Returns free memory

## Implementation Details

Located in [src/ast-modules/builtins.c](../src/ast-modules/builtins.c) at the `builtin_function` function, VARPTR creates a hash value from the variable name characters and offsets it into the upper half of the simulated memory space. This allows multiple variables to have unique identifiable addresses without requiring a full symbol table lookup.
