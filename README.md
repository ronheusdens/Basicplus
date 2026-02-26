# TRS-80 BASIC Interpreter - AST Implementation
**Version 1.05** - February 2026

A modern, AST-based implementation of a TRS-80 Level II BASIC interpreter written in C, featuring a clean modular architecture with separate lexing, parsing, and execution phases.

## Overview
This project implements a full-featured BASIC interpreter using an Abstract Syntax Tree (AST) approach, providing better code organization, maintainability, and extensibility compared to traditional single-pass interpreters.

### Key Features
- **AST-Based Architecture**: Clean separation between lexical analysis, parsing, and execution
- **Interactive REPL**: Line-by-line program entry with immediate mode execution
- **SDL2 Windowed Terminal**: 80×24 authentic TRS-80 interface with scrollback and line editing (gracefully falls back to stdio if SDL unavailable)
- **INKEY$**: Non-blocking keyboard input for real-time programs
- **INPUT Prompts**: INPUT "prompt" var syntax supported
- **TRS-80 Compatible Error Codes**: Full compliance with official TRS-80 Level II BASIC error codes (23 error types)
- **Parser Error Line Numbers**: Parse errors include source line numbers
- **Type Mismatch Detection**: Proper type checking for string operations with error code 13
- **Error Handling**: Complete ON ERROR GOTO/RESUME support with ERR and ERL variables
- **String Concatenation**: Full support with type checking (string + string only)
- **Multi-Statement Lines**: Multiple statements per line with colon separators
- **File I/O**: Sequential and random access file operations
- **Type Declarations**: DEFINT, DEFSNG, DEFDBL, DEFSTR for variable type defaults
- **Rich Function Library**: Math, string, and utility functions
- **SLEEP Statement**: Precise timing control with fractional second support
- **Array Support**: Multi-dimensional arrays with DIM statement
- **Data Statements**: READ/DATA/RESTORE for static data
- **Control Structures**: FOR/NEXT, GOSUB/RETURN, ON GOTO/GOSUB
- **WHILE/WEND**: Looping with nested and complex conditions
- **Video Memory**: 80x24 VRAM with POKE/PEEK and PRINT@ support
- **Block Glyph Mapping**: SDL renderer maps block/shade bytes to UTF-8 glyphs

## Architecture
```
┌─────────────────────────────────────────────────────────┐
│                     Source Code                         │
└─────────────────┬───────────────────────────────────────┘
                  │
                  ▼
          ┌───────────────┐
          │    Lexer      │  Tokenization
          └───────┬───────┘
                  │
                  ▼
          ┌───────────────┐
          │    Parser     │  AST Construction
          └───────┬───────┘
                  │
                  ▼
          ┌───────────────┐
          │  Symbol Table │  Type Analysis
          └───────┬───────┘
                  │
                  ▼
          ┌───────────────┐
          │   Executor    │  Runtime Execution
          └───────┬───────┘
                  │
                  ▼
          ┌───────────────┐
          │    Runtime    │  Variable Storage
          └───────────────┘
```

## Building

### Prerequisites
- C compiler (clang or gcc)
- Make
- macOS or Linux
- SDL2 (optional, for windowed terminal interface)

### Build Commands
```bash
# Build AST-based interpreter (primary & recommended)
make ast-build

# Run test suite
make test

# Build macOS app bundle (macOS only)
make install-app

# Clean build artifacts
make clean
```

The AST interpreter binary will be at: `build/bin/basic-trs80-ast`

## Usage

### Interactive Mode
```bash
./build/bin/basic-trs80-ast
```

Interactive commands:
- `NEW` - Clear program in memory
- `VERSION` - Shows build datetime
- `LIST` - Display program
- `RUN` - Execute program
- `LOAD "filename"` - Load program from file
- `SAVE "filename"` - Save program to file
- `AUTO [start[,increment]]` - Auto line numbering
- `SYSTEM`, 'EXIT' - Exit interpreter

### Running Programs
```bash
./build/bin/basic-trs80-ast program.bas
```

### Demo Apps
- [apps/pong.bas](apps/pong.bas) - Pong with INKEY$ and video memory
- [apps/clock.bas](apps/clock.bas) - Clock with INPUT prompts
- [apps/radius.bas](apps/radius.bas) - Radius calculator with INPUT prompts
- [apps/binary.bas](apps/binary.bas) - Binary converter

### Example Programs
```basic
10 REM Fibonacci sequence
20 A = 0: B = 1
30 FOR I = 1 TO 10
40   PRINT A;
50   C = A + B
60   A = B: B = C
70 NEXT I
80 END
```

```basic
10 REM Error handling example
20 ON ERROR GOTO 100
30 PRINT "Enter a number:"
40 INPUT X
50 PRINT "Result: "; 10 / X
60 END
100 PRINT "Error "; ERR; " at line "; ERL
110 RESUME NEXT
```

## Language Features

### Data Types

- **Numeric**: Double precision floating point (default)
  - Integer suffix: `%` (32-bit signed)
  - Single suffix: `!` (32-bit float)
  - Double suffix: `#` (64-bit float)
- **String**: Variable-length strings with `$` suffix [MAXSIZE=127 bytes]

### Type Declarations
```basic
DEFINT I-N      ' Variables I through N default to integer
DEFSNG A-H      ' Variables A through H default to single
DEFDBL O-Z      ' Variables O through Z default to double
DEFSTR S        ' Variable S defaults to string
```

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `^` (power), `MOD`
- Relational: `=`, `<>`, `<`, `>`, `<=`, `>=`
- Logical: `AND`, `OR`, `NOT`

### Control Flow
```basic
IF condition THEN statement [ELSE statement]
FOR var = start TO end [STEP increment]
WHILE condition
WEND
GOSUB line_number ... RETURN
ON expression GOTO line1, line2, ...
ON ERROR GOTO line_number
```

### Built-in Functions
**Math Functions:**
- `SIN(x)`, `COS(x)`, `TAN(x)`, `ATN(x)`
- `LOG(x)`, `EXP(x)`, `SQR(x)`, `ABS(x)`
- `INT(x)`, `SGN(x)`, `RND(x)`

**String Functions:**
- `LEN(s$)`, `LEFT$(s$,n)`, `RIGHT$(s$,n)`, `MID$(s$,start[,length])`
- `ASC(s$)`, `CHR$(n)`, `STR$(n)`, `VAL(s$)`
- `INSTR([start,]s1$,s2$)`, `STRING$(n,char$)`

**File Functions:**
- `LOC(n)` - Current file position
- `LOF(n)` - Length of file
- `EOF(n)` - End of file test

### File I/O
```basic
OPEN "filename" FOR INPUT AS #1
INPUT #1, A$, B
CLOSE #1

OPEN "filename" FOR OUTPUT AS #2
PRINT #2, "Hello World"
CLOSE #2
```

### Error Handling
```basic
ON ERROR GOTO 1000
' ... code that might error ...
END

1000 REM Error handler
1010 PRINT "Error "; ERR; " at line "; ERL
1020 RESUME NEXT    ' Continue at next line
1030 RESUME 100     ' Continue at specific line
1040 RESUME         ' Retry same line
```

**TRS-80 Standard Error Codes:**
- 1: NEXT without FOR
- 2: Syntax error
- 3: Return without GOSUB
- 4: Out of data
- 5: Illegal function call
- 6: Overflow
- 7: Out of memory
- 8: Undefined line
- 9: Subscript out of range
- 10: Redimensioned array
- 11: Division by zero
- 12: Illegal direct
- 13: Type mismatch
- 14: Out of string space
- 15: String too long
- [Additional codes 16-23 supported]

## Project Structure
```
.
├── src/
│   ├── ast-modules/           # AST-based interpreter (modular)
│   │   ├── ast.c/h                # AST node definitions
│   │   ├── ast_helpers.c          # AST helper routines
│   │   ├── builtins.c/h           # Built-in functions
│   │   ├── common.c/h             # Shared utilities
│   │   ├── compat.c/h             # Compatibility shims
│   │   ├── debug.c/h              # Debugging utilities
│   │   ├── errors.c/h             # Error handling
│   │   ├── eval.c/h               # Expression evaluation
│   │   ├── executor.c/h           # Program execution
│   │   ├── lexer.c/h              # Tokenization
│   │   ├── main.c                 # REPL & CLI entry
│   │   ├── parser.c/h             # AST construction
│   │   ├── runtime.c/h            # Variable storage
│   │   ├── symtable.c/h           # Symbol table
│   │   ├── termio.c/h             # Terminal I/O (stdio backend)
│   │   ├── termio_sdl.c           # SDL2 terminal/graphics backend
│   │   ├── video_backend.c/h      # Video memory backend abstraction
│   │   ├── videomem.c/h           # Video memory implementation
│   │   ├── *_old.c                # Legacy/experimental modules
│   │   └── *.bak, *_backup.c      # Backups (not used in build)
├── tests/
│   └── basic_tests/           # Test suite
├── docs/                      # Documentation
├── build/bin/                 # Compiled binaries
└── Makefile
```

## Tests
```bash
# Run full test suite (macOS-friendly)
cd tests/basic_tests
./testbench.sh

# Run individual test
./build/bin/basic-trs80-ast tests/basic_tests/01_trig.bas
```

Test coverage includes:
- Basic I/O (PRINT, INPUT)
- Arithmetic and string operations
- String concatenation with type checking
- Multi-statement lines with colon separators
- Control structures (IF, FOR, WHILE/WEND, GOSUB)
- Arrays and multi-dimensional arrays
- DATA/READ/RESTORE statements
- File I/O operations
- Error handling with TRS-80 standard error codes (including Out of data)
- Built-in functions (math, string, file)
- Power operator (^)
- Type mismatch detection

## Development

### Adding New Features
1. **Add token** (if new keyword): Update `lexer.h` and `lexer.c`
2. **Add AST node**: Update `ast.h` with new `StmtType` or `ExprType`
3. **Update parser** (if new syntax): Add case to `parse_statement()` in `parser.c`
4. **Update executor** (if new execution logic): Add case to `execute_stmt()` in `executor.c`
5. **Add tests**: Create `.bas` test file in `tests/basic_tests/`

### Architecture Notes
- **No legacy code**: Legacy monolith interpreter (basic-trs80) removed as of Feb 2026
- **Modular design**: Each component has clear responsibilities and minimal coupling
- **Symbol table**: Performs type analysis before execution
- **Two I/O backends**: stdio (automatic fallback) and SDL2 (windowed interface)
3. **Add parser**: Implement parse function in `parser.c`
4. **Add executor**: Implement execution logic in `executor.c`
5. **Add test**: Create test file in `tests/basic_tests/`
TRS-80 Level II BASIC with official error codes and extensions:

**Implemented:**
- Core BASIC statements (LET, PRINT, INPUT, IF/THEN/ELSE, FOR/NEXT, GOSUB/RETURN)
- TRS-80 official error codes (23 types)
- String concatenation with type checking
- Multi-statement lines (colon separators)
- Error handling (ON ERROR, RESUME, ERR, ERL) with correct error codes
- File I/O (sequential and random access, OPEN, CLOSE, INPUT#, PRINT#)
- Type declarations (DEFINT, DEFSNG, DEFDBL, DEFSTR)
- Arrays (single and multi-dimensional with DIM)
- DATA/READ/RESTORE statements
- ON GOTO/GOSUB statements
- PEEK/POKE for machine code interface
- USR function for machine code calls with DEFUSR
- SLEEP statement
- Math functions (SIN, COS, TAN, ATN, LOG, EXP, SQR, ABS, INT, SGN, RND)
- String functions (LEFT$, RIGHT$, MID$, LEN, ASC, CHR$, STR$, VAL, INSTR, STRING$)
- WHILE/WEND flow control logic added (as of V1.05)
- Graphics commands SET, RESET, CLS, POINT, LINE, SQUARE, CIRCLE
- Sound (SOUND command)

## Version History
### V.1.4.7. (Feb 21, 2026)
- Added RND(), ABS(), INT(), SGN()
- Added ' apostrophe for REM
- Added INSTR substring lookup

### V.1.4.6. (Feb 20, 2026)
- Completed testbench (tests/basic_tests/*)

### V1.4.3 (Feb 15, 2026)
- Added RESUME
- Enhanced make test where it is now possible to run individual test 'make test TEST=nn'
- consolidated version: old legacy monolith (basic.c) removed from project

### V1.3.2 (Feb 14, 2026)
- Added RENUM (start) (step)
- Added RESTORE (linenumber)
  
### v1.3.1 (February 12, 2026)
- Fixed: LINE statement now requires syntax LINE x1,y1,x2,y2 (no parentheses or dashes)
- Added: Comprehensive graphics test (47, 48) added to testbench
- Improved: Documentation and error messages for graphics statements

### v1.05 (February 10, 2026)
- INKEY$ support for non-blocking input
- INPUT prompt strings (INPUT "prompt" var)
- Parser errors include line numbers
- SDL renderer maps block/shade bytes to UTF-8 glyphs
- Demo apps updated and Pong added

### v1.02 (February 5, 2026)
- Implemented type mismatch detection for string operations
- Updated all error codes to match official TRS-80 Level II BASIC specification
- Fixed error code propagation through execution stack
- Fixed test suite whitespace handling for proper tab formatting
- Added comprehensive string concatenation tests
- All 38 tests passing

### v1.0 (Previous)
- Complete multi-statement support
- String concatenation implementation
- Multi-dimensional array support
- Power operator (^)

## License
[Add your license here]

## Authors
Ron Heusdens

## Acknowledgments
Based on TRS-80 Level II BASIC specifications and Radio Shack documentation.
