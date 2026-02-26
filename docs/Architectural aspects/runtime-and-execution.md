## Runtime, Execution, and Error Handling

This document describes the runtime system, statement execution, variable storage, and error handling in the interpreter.

### Runtime System
- Manages variable storage and program state.
- Variables auto-create on first reference; type determined by DEF* or suffix.

### Executor
- Walks the AST and executes statements.
- Handles GOTO, GOSUB, FOR/NEXT, etc.

### Variable Storage
- Supports numeric, string, and array variables.
- Arrays must be DIM'd before use.

### Error Handling
- ON ERROR GOTO, ERR, ERL, RESUME, RESUME NEXT.
- runtime_set_error() sets error state.
- See ERROR_HANDLING_REFERENCE.md for error codes.