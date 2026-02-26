## Interpreter Architecture Overview

This document describes the overall architecture of the TRS-80 Level II BASIC interpreter, including its modular AST-based design, file/module responsibilities, and build system/platform detection.

### Modular AST Design
- Lexer: Tokenizes source text (lexer.c/h)
- Parser: Builds AST from tokens (parser.c/h)
- AST: Node definitions (ast.c/h)
- Runtime: Variable storage/state (runtime.c/h)
- Executor: Statement execution (executor.c/h)
- Builtins: Built-in functions (builtins.c/h)
- Terminal I/O: termio.c/h (stdio), termio_sdl.c (SDL2)
- Video Memory: videomem.c/h (80x24 VRAM)
- Error Handling: errors.c/h

### File/Module Responsibilities
- Each module is focused and testable.
- See COMPONENT_INTERFACES.md for details.

### Build System & Platform Detection
- Makefile auto-detects OS/arch (Darwin/Linux, arm64/x86_64).
- SDL2 detection for graphical terminal.
- Build artifacts in build/bin/.

### Project Organization
- See PROJECT_ORGANIZATION.md for directory structure and conventions.