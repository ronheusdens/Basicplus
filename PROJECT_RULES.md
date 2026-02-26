# Project Rules - TRS-80 BASIC (AST Implementation)

## Workspace Context
- Workspace root: /Users/ronheusdens/Stack/Dev/MacSilicon/c/basic
- Primary OS: macOS (arm64)
- Legacy monolith interpreter (src/basic-trs80/) removed Feb 2026

## AST Interpreter (Primary)
- Build: src/ast-modules/ -> make ast-build -> build/bin/basic-trs80-ast
- Single interpreter for all development

## AST Architecture
- Lexer: src/ast-modules/lexer.c/.h
- Parser: src/ast-modules/parser.c/.h
- AST: src/ast-modules/ast.c/.h, ast_helpers.c
- Runtime: src/ast-modules/runtime.c/.h
- Executor: src/ast-modules/executor.c
- Builtins: src/ast-modules/builtins.c
- Errors: src/ast-modules/errors.c/.h
- Terminal/SDL: src/ast-modules/termio_sdl.c
- Video memory: src/ast-modules/videomem.c/.h (80x24, base 0x3C00)

## Build and Test
- Build: make ast-build
- Test: make test
- Test single: make test TEST=49
- Tests: tests/basic_tests/ (50+ cases)
- macOS does not provide GNU timeout by default

## Input and Output
- Supports INKEY$ (non-blocking)
- Supports INPUT "prompt" var in parser and executor
- Parser error messages include line numbers
- SDL output uses UTF-8; raw bytes > 127 need mapping

## Video Memory
- VRAM: 0x3C00 to 0x437F (80x24)
- POKE to VRAM updates screen

## SDL Glyph Mapping
- Renderer maps block/shade bytes to UTF-8
- 0xDB -> U+2588 full block
- 0xFE -> U+25A0 black square
- 0xB0..0xB2 -> light/medium/dark shade

## Coding Style
- C style: kernel braces, 4-space indent
- Keep comments minimal and useful
- Avoid non-ASCII in source unless necessary
- Do not revert unrelated changes in a dirty tree

## Recent Changes
- Removed legacy monolith interpreter (basic-trs80/basic.c)
- Removed legacy build targets (make basic, make basic-trs80, make basic-trs80-curses)
- Removed test-legacy target
- Consolidated all development to AST interpreter
- Updated README.md and Makefile for AST-only workflow
- Added DELETE statement (single and range deletion)

## Recent Context
- Parser: line numbers appended to error messages
- Executor: INPUT prompt displayed correctly
- SDL: UTF-8 glyph mapping added for block chars
- Apps updated for INPUT prompts
- Pong demo in apps/pong.bas using VRAM and INKEY$
