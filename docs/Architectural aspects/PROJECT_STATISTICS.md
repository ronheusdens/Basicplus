# TRS-80 Level II BASIC Interpreter - Project Statistics

**Generated:** February 25, 2026  
**Architecture:** AST-based modular design  
**Total Functions:** 354  
**Total Modules:** 18 (.c files)

---

## Module Function Overview

| Module | Functions | Key Responsibility |
|--------|:---------:|-------------------|
| parser.c | 80 | BASIC statement & expression parsing |
| executor.c | 55 | Statement execution engine |
| runtime.c | 50 | Variable storage, arrays, error handling |
| main.c | 21 | REPL, file I/O, program management |
| termio_sdl.c | 25 | SDL2 graphics rendering, terminal UI |
| compat.c | 12 | TRS-80 compatibility checking |
| termio.c | 14 | ANSI terminal I/O fallback |
| video_backend.c | 11 | Graphics primitives (line, circle, pixel) |
| symtable.c | 11 | Symbol table & analysis |
| eval.c | 10 | Expression evaluation |
| videomem.c | 9 | Video memory management (0x3C00) |
| lexer.c | 9 | Tokenization from BASIC source |
| builtins.c | 6 | Built-in functions (math, string, I/O) |
| common.c | 7 | Memory management wrappers |
| ast.c | 13 | AST node creation and manipulation |
| ast_helpers.c | 5 | AST helper functions |
| debug.c | 4 | Debug utilities |
| errors.c | 2 | Error message handling |
| **TOTAL** | **354** | |

---

## Detailed Function Listing by Module

### common.c (7 functions)
Memory utilities and platform detection.

| Function | Purpose |
|----------|---------|
| `xmalloc` | Safe memory allocation with error checking |
| `xcalloc` | Safe zeroed allocation |
| `xrealloc` | Safe memory reallocation |
| `xstrdup` | Safe string duplication |
| `platform_name` | Get platform identifier |
| `arch_name` | Get architecture identifier |
| `error_exit` | Fatal error handler |

---

### ast.c (13 functions)
Abstract Syntax Tree node management and printing.

| Function | Purpose |
|----------|---------|
| `ast_expr_create` | Create expression node |
| `ast_stmt_create` | Create statement node |
| `ast_program_create` | Create program structure |
| `ast_expr_free` | Free expression tree |
| `ast_expr_copy` | Deep copy expression |
| `ast_stmt_free` | Free statement node |
| `ast_program_free` | Free entire program |
| `print_indent` | Utility for debug output |
| `ast_expr_print` | Print expression debug info |
| `ast_stmt_print` | Print statement debug info |
| `ast_program_print` | Print program structure |
| `stmt_type_name` | Get statement type name |
| `ast_execute_stmt` | Legacy execution dispatch |

---

### ast_helpers.c (5 functions)
AST node helper utilities.

| Function | Purpose |
|----------|---------|
| `ast_expr_add_child` | Add child to expression |
| `ast_stmt_add_expr` | Add expression to statement |
| `ast_stmt_set_body` | Set statement body |
| `ast_program_add_line` | Add line to program |
| `ast_program_line_create` | Create program line |

---

### lexer.c (9 functions)
Tokenization of BASIC source code.

| Function | Purpose |
|----------|---------|
| `lookup_keyword` | Check if token is keyword |
| `add_token` | Add token to array |
| `skip_whitespace` | Skip whitespace in source |
| `lex_number` | Parse numeric literal |
| `lex_string` | Parse string literal |
| `lex_identifier` | Parse identifier/keyword |
| `lexer_tokenize` | Main tokenization function |
| `lexer_create` | Initialize lexer |
| `lexer_free` | Cleanup lexer |

---

### parser.c (80 functions)
BASIC language parsing into AST.

**Key Functions:**
- `parse_program` - Entry point for parsing
- `parse_statement` - Main statement dispatcher
- `parse_print_stmt` - PRINT statement
- `parse_input_stmt` - INPUT statement
- `parse_for_stmt` - FOR loop
- `parse_if_stmt` - IF-THEN-ELSE
- `parse_gosub_stmt` - GOSUB statement
- `parse_case_stmt` - CASE-WHEN-ENDCASE
- `parse_merge_stmt` - MERGE statement
- `parse_delete_stmt` - DELETE statement
- `parse_do_loop_stmt` - DO-LOOP statement
- `parse_expression` - Expression parsing
- `parse_primary` - Primary expression
- **+66 more specialized parser functions**

| Category | Count |
|----------|-------|
| Statement parsers | ~35 |
| Expression parsers | ~25 |
| Utility/helper | ~20 |

---

### symtable.c (11 functions)
Symbol table and type analysis.

| Function | Purpose |
|----------|---------|
| `get_var_type_from_name` | Determine variable type by name |
| `set_type_range` | Register type range (DEFINT, etc.) |
| `analyze_program_line` | Analyze line for symbols |
| `analyze_statement` | Analyze statement for symbols |
| `analyze_expression` | Analyze expression for symbols |
| `symtable_create` | Initialize symbol table |
| `symtable_free` | Cleanup symbol table |
| `symtable_lookup` | Search for symbol |
| `symtable_insert` | Add symbol to table |
| `symtable_insert_array` | Register array declaration |
| `symtable_analyze_program` | Full program analysis |

---

### runtime.c (50 functions)
Runtime state management, variables, arrays, and error handling.

**Core Categories:**
- **Variable Management** (15 functions) - Get/set variables, type checking
- **Array Operations** (12 functions) - Array access, bounds checking
- **Error Handling** (8 functions) - Error state, codes, messages
- **File I/O State** (8 functions) - File handles, sequential/random access
- **Program State** (5 functions) - Variables clear, reset, DEF ranges
- **Callbacks** (2 functions) - MERGE, SAVE, DELETE callbacks

| Function Category | Count |
|-------------------|-------|
| Variable access | 15 |
| Array management | 12 |
| Error handling | 8 |
| File I/O | 8 |
| State management | 5 |
| Callbacks | 2 |

---

### eval.c (10 functions)
Expression evaluation and type coercion.

| Function | Purpose |
|----------|---------|
| `eval_expr_is_string` | Check if expression evaluates to string |
| `eval_numeric_expr` | Evaluate to numeric value |
| `eval_string_expr` | Evaluate to string value |
| `eval_condition` | Evaluate boolean condition |
| `eval_is_true` | Check if value is true |
| `ast_eval_expr` | Main evaluation dispatch |
| `eval_expr_internal` | Internal numeric evaluation |
| `eval_string_expr_internal` | Internal string evaluation |
| `is_string_variable` | Check variable type |
| `is_string_expr` | Check expression type |

---

### executor.c (55 functions)
Statement execution engine and control flow.

**Core Categories:**
- **Control Flow** (15 functions) - IF, WHILE, FOR, GOSUB, GOTO, RETURN
- **I/O Operations** (12 functions) - PRINT, INPUT, LINE INPUT, PRINT AT
- **Variable/Array** (8 functions) - LET, DIM, READ, DATA
- **Blocks/Structures** (10 functions) - DO-LOOP, CASE-WHEN, nested statements
- **Special** (10 functions) - ERROR handling, MERGE, DELETE, SAVE

| Function Category | Count |
|-------------------|-------|
| Control flow | 15 |
| I/O operations | 12 |
| Variables/arrays | 8 |
| Blocks/structures | 10 |
| Special features | 10 |

---

### builtins.c (6 functions)
Built-in function implementation and terminal control.

| Function | Purpose |
|----------|---------|
| `restore_inkey_terminal` | Restore terminal after INKEY$ |
| `setup_inkey_raw_mode` | Setup raw mode for INKEY$ |
| `get_numeric_arg` | Extract numeric argument |
| `get_string_arg` | Extract string argument |
| `call_numeric_function` | Dispatch numeric functions |
| `call_string_function` | Dispatch string functions |

---

### debug.c (4 functions)
Debug utilities and state printing.

| Function | Purpose |
|----------|---------|
| `debug_create` | Initialize debugger |
| `debug_free` | Cleanup debugger |
| `debug_print_state` | Print interpreter state |
| `debug_print_variable` | Print variable value |

---

### errors.c (2 functions)
Error message handling.

| Function | Purpose |
|----------|---------|
| `error_message` | Get error message text |
| `error_print` | Print formatted error |

---

### compat.c (12 functions)
TRS-80 compatibility checking and violation tracking.

| Function | Purpose |
|----------|---------|
| `compat_init` | Initialize checker |
| `compat_free` | Cleanup |
| `compat_record_violation` | Log compatibility issue |
| `compat_is_strict` | Check strict mode |
| `compat_print_violations` | List all violations |
| `compat_clear_violations` | Reset violation log |
| `compat_is_trs80_keyword` | Check if TRS-80 keyword |
| `compat_is_trs80_function` | Check if TRS-80 function |
| `check_array_usage` | Verify array compatibility |
| `collect_dims` | Gather dimension info |
| `check_statement` | Check statement compatibility |
| `compat_check_program_arrays` | Full program check |

---

### videomem.c (9 functions)
Video memory management and display buffer.

| Function | Purpose |
|----------|---------|
| `videomem_init` | Initialize video memory |
| `videomem_peek` | Read byte from video RAM |
| `videomem_poke` | Write byte to video RAM |
| `videomem_write_char` | Write character with attributes |
| `videomem_get_buffer` | Get pointer to video buffer |
| `videomem_get_char` | Read character from buffer |
| `videomem_scroll_up` | Scroll display up |
| `videomem_clear` | Clear video buffer |
| `videomem_print_debug` | Debug print utilities |

---

### video_backend.c (11 functions)
Graphics primitives and rendering.

| Function | Purpose |
|----------|---------|
| `video_clear` | Clear graphics mode |
| `video_set_pixel` | Set individual pixel |
| `video_get_pixel` | Read pixel |
| `video_set_color` | Set drawing color |
| `video_set_background` | Set background color |
| `video_draw_line` | Draw line |
| `video_draw_circle` | Draw circle |
| `video_paint` | Fill region |
| `video_set_screen_mode` | Set display mode |
| `video_get_screen_mode` | Query current mode |
| `video_graphics_active` | Check graphics state |

---

### termio_sdl.c (25 functions)
SDL2-based graphics and terminal rendering.

| Function | Purpose |
|----------|---------|
| `render_video_memory` | Render video buffer to SDL |
| `choose_font_path` | Locate system font |
| `glyph_for_byte` | Map character to glyph |
| `is_solid_block` | Check if block character |
| `init_lines` | Initialize line buffer |
| `ensure_line` | Allocate line |
| `put_char_at` | Write character position |
| `newline_advance` | Handle newline |
| `render_text` | Render text output |
| `termio_init` (SDL) | Initialize SDL terminal |
| `termio_shutdown` (SDL) | Cleanup SDL |
| `termio_clear` (SDL) | Clear SDL display |
| `termio_write` (SDL) | Write to SDL terminal |
| **+12 additional termio functions** | |

---

### termio.c (14 functions)
ANSI terminal fallback I/O (when SDL not available).

| Function | Purpose |
|----------|---------|
| `is_tty_mode` | Check if terminal mode |
| `termio_init` (ANSI) | Initialize terminal |
| `termio_shutdown` (ANSI) | Cleanup |
| `termio_clear` (ANSI) | Clear screen |
| `termio_write` (ANSI) | Write text |
| `termio_write_char` (ANSI) | Write single character |
| `termio_put_char_at` (ANSI) | Position and write |
| `termio_printf` (ANSI) | Formatted output |
| `termio_present` (ANSI) | Flush output |
| `termio_readline` (ANSI) | Read line input |
| `termio_poll_key` (ANSI) | Check for key |
| `termio_set_title` (ANSI) | Set window title |
| `termio_render_graphics` (ANSI) | Graphics fallback |
| `termio_beep` (ANSI) | Sound output |

---

### main.c (21 functions)
REPL, file I/O, and program management.

| Function | Purpose |
|----------|---------|
| `handle_sigint` | Ctrl-C signal handler |
| `find_line_index` | Binary search for line number |
| `insert_line` | Add/update program line |
| `clear_program` | Clear all program lines |
| `match_keyword` | Case-insensitive keyword matching |
| `replace_all_line_refs` | Update line references during RENUM |
| `do_renum` | Renumber program lines |
| `edit_line` | Edit single program line |
| `list_program` | Display program listing |
| `build_program_text` | Reconstruct source from AST |
| `parse_filename_arg` | Parse file path argument |
| `load_program_file` | Load .BAS file |
| `save_program_file` | Save .BAS file |
| `save_callback` | Callback for SAVE statement |
| `delete_callback` | Callback for DELETE statement |
| `merge_callback` | Callback for MERGE statement |
| `run_program_text` | Execute program |
| `run_program_text_from_line` | Execute from line number |
| `run_interactive` | Interactive REPL mode |
| `main` | Program entry point |
| **+1 utility function** | |

---

## Code Metrics Summary

### Distribution by Module Type

| Type | Modules | Functions | % |
|------|---------|-----------|---|
| **Core Parsing** | 2 | 89 | 25% |
| **Execution** | 2 | 65 | 18% |
| **Runtime Support** | 3 | 68 | 19% |
| **I/O & Terminal** | 3 | 48 | 14% |
| **Graphics/Video** | 3 | 31 | 9% |
| **Utilities** | 4 | 20 | 6% |
| **Compatibility** | 1 | 12 | 3% |
| **Error Handling** | 1 | 2 | 1% |

### Complexity Distribution

| Complexity | Modules | Function Ranges |
|------------|---------|-----------------|
| **Very High** | parser.c | 80 functions |
| **High** | executor.c, runtime.c | 50-55 functions |
| **Medium** | main.c, termio_sdl.c | 20-25 functions |
| **Low** | Most utilities | 2-14 functions |

---

## Architecture Insights

### Modular Design Pattern
The interpreter follows a clean separation of concerns:

1. **Input Layer**: `lexer.c` → tokenization
2. **Parsing Layer**: `parser.c` → AST construction
3. **Analysis Layer**: `symtable.c`, `compat.c` → semantic checking
4. **Execution Layer**: `executor.c` + `eval.c` → runtime behavior
5. **Runtime Support**: `runtime.c` → state management
6. **Output Layer**: `termio.c`/`termio_sdl.c` + `videomem.c`/`video_backend.c` → display
7. **Management**: `main.c` → REPL and file I/O

### Critical Modules (by function count and importance)

1. **parser.c** (80) - Most complex, handles all language syntax
2. **executor.c** (55) - Controls program flow and statement execution
3. **runtime.c** (50) - Manages all runtime state and variables
4. **termio_sdl.c** (25) - Primary UI for graphical mode
5. **main.c** (21) - REPL and persistent state

### Scalability Notes

- **Total implementation: 354 functions** across 18 modules
- **Average functions per module: ~20** (excluding parser outlier)
- **Focused modules** ensure maintainability
- **Parser concentration** reflects language complexity
- **Execution well-distributed** between executor and runtime

---

## Recent Additions (v1.4.8)

- **CASE-WHEN-ENDCASE** statement (tests 69-73)
- **MERGE** statement for program merging (tests 74)
- **DO-LOOP** statement support
- Expression copying for CASE value matching

---

**Last Updated**: February 25, 2026
