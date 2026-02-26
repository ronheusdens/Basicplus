# Machine Code Support (USR, DEFUSR, VARPTR)

## Overview

The TRS-80 BASIC interpreter now supports machine code simulation, mimicking the behavior of the original TRS-80 Level II BASIC's machine language interface. While the original would execute actual Z80 machine code, this implementation provides a simulated environment for testing and development.

## Memory Architecture

### Memory Size Configuration

At startup, the interpreter prompts:
```
Memory size?
```

- **ENTER** (default): 32768 bytes (32KB)
- **Valid range**: 1 to 32768 (must be ≤ 32768)
- **Invalid input**: Defaults to 32768 with error message

This mimics the original TRS-80 behavior where the user could specify how much memory to reserve for BASIC programs, with the remainder available for machine code.

### Memory Layout

```
Address 0                              basic_memory_size (32768 default)     65535
├──────────────────────────────────────┼────────────────────────────────────┤
│   Machine Code Area                  │    BASIC Program Area               │
│   (USR routines, variables)          │    (program lines, data)            │
└──────────────────────────────────────┴────────────────────────────────────┘
```

- **0 to (basic_memory_size-1)**: Machine code area (accessible via USR)
- **basic_memory_size to 65535**: BASIC program area

## Commands and Functions

### DEFUSR = address

Sets the default address for USR() calls.

**Syntax:**
```basic
DEFUSR = address
```

**Parameters:**
- `address`: Integer from 0 to (basic_memory_size-1)

**Example:**
```basic
10 DEFUSR = 1000
20 REM Now USR() will call machine code at address 1000
```

**Errors:**
- `?ILLEGAL FUNCTION CALL` - Address out of range

### USR([address])

"Executes" machine code at the specified address (or default DEFUSR address).

**Syntax:**
```basic
result = USR()           REM Use DEFUSR address
result = USR(address)    REM Use explicit address
```

**Parameters:**
- `address` (optional): Integer from 0 to (basic_memory_size-1)
- If omitted, uses address set by DEFUSR

**Returns:**
- Simulated return value (byte at the address)
- In a real implementation, this would be the Z80 A register after execution

**Example:**
```basic
10 POKE 1000, 42        : REM Store "machine code"
20 DEFUSR = 1000
30 X = USR()            : REM Returns 42
40 Y = USR(1001)        : REM Returns byte at 1001
```

**Errors:**
- `?ILLEGAL FUNCTION CALL` - Address out of range

### VARPTR(variable)

Returns the simulated memory address of a variable.

**Syntax:**
```basic
address = VARPTR(variable)
```

**Parameters:**
- `variable`: Any variable name (numeric or string)

**Returns:**
- Simulated memory address (hash-based in upper half of machine code area)
- Range: (basic_memory_size/2) to (basic_memory_size-1)

**Example:**
```basic
10 A = 100
20 ADDR = VARPTR(A)
30 PRINT "A is at address"; ADDR
```

**Notes:**
- In the original TRS-80, this returned the actual RAM address
- Here it returns a consistent hash value for each variable name
- Useful for understanding memory layout concepts

## Implementation Details

### Simulation vs. Real Execution

**Original TRS-80 Behavior:**
- USR() would JSR (Jump to Subroutine) to Z80 machine code
- Code would execute on actual hardware
- Return via RET instruction with value in A register

**This Implementation:**
- USR() returns the byte value at the specified address
- No actual code execution (would require Z80 emulator)
- Safe simulation for testing and education

### Use Cases

1. **Education**: Learn about machine code interfaces without hardware
2. **Testing**: Verify BASIC programs that call machine routines
3. **Development**: Prototype TRS-80 programs before deploying to real hardware
4. **Preservation**: Run historical TRS-80 programs that use USR

### Limitations

- No actual Z80 instruction execution
- USR simply returns memory byte (not execution result)
- Variable addresses are simulated (hash-based, not real pointers)
- No parameter passing beyond address
- No stack manipulation or register access

## Example Programs

### Example 1: Basic Machine Code Setup

```basic
10 REM Setup machine code routine
20 PRINT "Initializing machine code area..."
30 DEFUSR = 2000
40 REM Store "machine code" bytes
50 POKE 2000, 65   : REM 'A' character
60 POKE 2001, 66   : REM 'B' character
70 POKE 2002, 67   : REM 'C' character
80 REM Call routine
90 RESULT = USR()
100 PRINT "USR returned:"; RESULT
110 END
```

### Example 2: Variable Address Exploration

```basic
10 REM Explore variable addresses
20 A = 10
30 B$ = "HELLO"
40 DIM ARR(10)
50 PRINT "Variable addresses:"
60 PRINT "A at:"; VARPTR(A)
70 PRINT "B$ at:"; VARPTR(B$)
80 PRINT "ARR at:"; VARPTR(ARR)
90 END
```

### Example 3: Memory Configuration

```basic
10 REM This program tests memory limits
20 PRINT "Testing USR address validation..."
30 DEFUSR = 16384  : REM Valid (< 32768)
40 PRINT "USR at 16384: OK"
50 X = USR()
60 REM Attempting invalid address
70 DEFUSR = 40000  : REM Invalid (> 32768)
80 REM Should print error message
90 END
```

## Compatibility with Original TRS-80

### Identical Behavior:
- MEMORY? prompt at startup
- Default value: 32768
- Maximum value: 32768
- DEFUSR command syntax
- USR() function syntax
- VARPTR() function syntax
- Error messages

### Differences:
- No actual Z80 execution
- Variable addresses are simulated
- Full 64KB virtual memory (original had less due to ROM)
- No timing considerations
- No hardware I/O port access

## Testing

Test file: `tests/basic_tests/20_machine_code.bas`

Coverage:
- VARPTR address validity
- DEFUSR setting and validation
- Machine code area setup with POKE
- USR() with default address
- USR(addr) with explicit address
- Multiple variable address queries
- DEFUSR address changes

Run tests:
```bash
make test
```

## References

- TRS-80 Level II BASIC Reference Manual, Radio Shack, 1979
- Z80 Assembly Language Programming, Lance Leventhal, 1979
- TRS-80 Technical Reference Manual, Radio Shack, 1980
