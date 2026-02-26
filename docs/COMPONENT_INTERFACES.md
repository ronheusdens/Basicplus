# AST Architecture - Component Interfaces Reference

**Purpose:** Quick reference for component APIs and their relationships  
**Date:** February 1, 2026  
**Status:** Phase 1 Complete

---

## 1. COMMON (`common.h/c`)

### Purpose
Shared utilities, platform detection, memory management, error codes.

### Key Functions
```c
void *xmalloc(size_t size);              // Safe malloc
void *xcalloc(size_t count, size_t sz);  // Safe calloc
void *xrealloc(void *ptr, size_t size);  // Safe realloc
char *xstrdup(const char *str);          // Safe strdup
const char *platform_name(void);         // "macOS" or "Linux"
const char *arch_name(void);             // "ARM64" or "x86_64"
void error_exit(int code, const char *fmt, ...);
```

### Configuration Macros
```c
MAX_STACK_DEPTH      // 1024
MAX_FILES            // 32
MAX_VARIABLES        // 10000
MAX_ARRAYS           // 1000
MAX_STRING_LENGTH    // 32768
```

### Dependencies
- None (base layer)

### Status
✅ Complete

---

## 2. ERRORS (`errors.h/c`)

### Purpose
Error codes and error message formatting.

### Key Functions
```c
const char *error_message(int error_code);
void error_print(int error_code, int line_number);
```

### Error Codes (Partial)
```c
BASIC_ERR_SYNTAX_ERROR              // 2
BASIC_ERR_ILLEGAL_QUANTITY          // 5
BASIC_ERR_UNDEFINED_LINE            // 8
BASIC_ERR_TYPE_MISMATCH             // 11
BASIC_ERR_DIVISION_BY_ZERO          // 12
BASIC_ERR_ILLEGAL_FUNCTION          // 32
// ... more in errors.h
```

### Dependencies
- common.h

### Status
✅ Complete

---

## 3. AST (`ast.h/c`)

### Purpose
Define Abstract Syntax Tree node types and utilities.

### Key Types
```c
typedef enum {
    STMT_PRINT, STMT_INPUT, STMT_LET, STMT_IF,
    STMT_FOR, STMT_NEXT, STMT_GOTO, STMT_GOSUB,
    STMT_RETURN, STMT_READ, STMT_DATA, STMT_DIM,
    // ... 20+ statement types
} StmtType;

typedef enum {
    EXPR_NUMBER, EXPR_STRING, EXPR_VARIABLE,
    EXPR_ARRAY_ACCESS, EXPR_BINARY_OP, EXPR_UNARY_OP,
    EXPR_FUNCTION_CALL, EXPR_CAST
} ExprType;

typedef enum {
    OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MOD,
    OP_POWER, OP_EQ, OP_NE, OP_LT, OP_LE,
    OP_GT, OP_GE, OP_AND, OP_OR, OP_NOT
} OpType;

typedef struct ASTExpr { ... } ASTExpr;
typedef struct ASTStmt { ... } ASTStmt;

typedef struct {
    int line_number;
    ASTStmt **statements;
    int num_statements;
} ProgramLine;

typedef struct {
    ProgramLine *lines;
    int num_lines;
} Program;
```

### Key Functions
```c
ASTExpr *ast_expr_create(ExprType type);
ASTStmt *ast_stmt_create(StmtType type);
Program *ast_program_create(void);
void ast_expr_free(ASTExpr *expr);
void ast_stmt_free(ASTStmt *stmt);
void ast_program_free(Program *prog);
void ast_expr_print(ASTExpr *expr);        // Debug printing
void ast_stmt_print(ASTStmt *stmt);
void ast_program_print(Program *prog);
const char *stmt_type_name(StmtType type);
const char *expr_type_name(ExprType type);
const char *op_type_name(OpType type);
```

### Dependencies
- common.h

### Status
✅ Complete (structures defined, utilities implemented)

---

## 4. LEXER (`lexer.h/c`)

### Purpose
Convert source text to token stream.

### Key Types
```c
typedef enum {
    TOK_EOF, TOK_NUMBER, TOK_STRING, TOK_IDENTIFIER,
    TOK_KEYWORD, TOK_PLUS, TOK_MINUS, TOK_STAR,
    TOK_SLASH, TOK_CARET, TOK_MOD, TOK_PERCENT,
    TOK_EQ, TOK_NE, TOK_LT, TOK_LE, TOK_GT, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET,
    TOK_COMMA, TOK_SEMICOLON, TOK_COLON, TOK_DOLLAR,
    TOK_HASH, TOK_AMPERSAND, TOK_AT, TOK_QUESTION,
    TOK_NEWLINE, TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line_number;
    int column_number;
} Token;

typedef struct {
    const char *input;
    int pos;
    int line;
    int column;
    Token *tokens;
    int num_tokens;
    int capacity;
} Lexer;
```

### Key Functions
```c
Lexer *lexer_create(const char *input);
void lexer_free(Lexer *lexer);
Token *lexer_tokenize(Lexer *lexer);
int lexer_token_count(Lexer *lexer);
Token lexer_peek(Lexer *lexer);
Token lexer_next(Lexer *lexer);
Token lexer_current(Lexer *lexer);
const char *token_type_name(TokenType type);
```

### Dependencies
- common.h

### Status
✅ Complete (400+ lines of tokenization logic)

---

## 5. PARSER (`parser.h/c`) - PHASE 2

### Purpose
Convert token stream to AST.

### Key Types
```c
typedef struct {
    Token *tokens;
    int num_tokens;
    int pos;
    int error_code;
    char *error_msg;
} Parser;
```

### Key Functions (To Be Implemented)
```c
Parser *parser_create(Token *tokens, int num_tokens);
void parser_free(Parser *parser);
Program *parse_program(Parser *parser);
ProgramLine *parse_line(Parser *parser);
ASTStmt *parse_statement(Parser *parser);
ASTExpr *parse_expression(Parser *parser);
int parser_has_error(Parser *parser);
const char *parser_error_message(Parser *parser);
```

### Responsibilities
- Parse line numbers and statements
- Handle all 30+ statement types
- Recursive descent expression parsing
- Operator precedence management
- Error reporting with line/column info

### Dependencies
- ast.h
- lexer.h

### Status
🚧 STUB (Phase 2)

---

## 6. SYMTABLE (`symtable.h/c`) - PHASE 2

### Purpose
Semantic analysis: track variable types, function signatures, array dimensions.

### Key Types
```c
typedef enum {
    VAR_UNDEFINED = 0,
    VAR_DOUBLE,
    VAR_INTEGER,
    VAR_STRING,
    VAR_SINGLE,
    VAR_LONG
} VarType;

typedef struct {
    char *name;
    VarType type;
    int is_array;
    int *dimensions;
    int num_dimensions;
    int is_function;
    int line_defined;
} Symbol;

typedef struct {
    Symbol *symbols;
    int num_symbols;
    int capacity;
} SymbolTable;
```

### Key Functions (To Be Implemented)
```c
SymbolTable *symtable_create(void);
void symtable_free(SymbolTable *table);
Symbol *symtable_lookup(SymbolTable *table, const char *name);
void symtable_insert(SymbolTable *table, const char *name, VarType type);
void symtable_insert_array(SymbolTable *table, const char *name,
                           VarType type, int *dimensions, int num_dims);
int symtable_analyze_program(SymbolTable *table, Program *prog);
```

### Responsibilities
- Walk AST collecting DIM declarations
- Track DEFINT, DEFSNG, DEFDBL, DEFSTR ranges
- Record function declarations (DEF FN)
- Enable type checking in executor

### Dependencies
- common.h
- ast.h

### Status
🚧 STUB (Phase 2)

---

## 7. RUNTIME (`runtime.h/c`) - PHASE 3

### Purpose
Manage execution state: variables, stacks, file handles, error context.

### Key Types
```c
typedef struct {
    // Variables
    Var *variables;
    
    // Call stack
    int *gosub_stack;
    int gosub_sp;
    int gosub_capacity;
    
    // FOR loop stack
    ForFrame *for_stack;
    int for_sp;
    int for_capacity;
    
    // Error handling
    int error_code;
    int error_line;
    int error_handler_line;
    
    // File I/O
    FileHandle *file_handles;
    int num_files;
    
    // Other state
    int should_exit;
} RuntimeState;
```

### Key Functions (To Be Implemented)
```c
RuntimeState *runtime_create(void);
void runtime_free(RuntimeState *state);
void runtime_set_variable(RuntimeState *state, const char *name, double value);
double runtime_get_variable(RuntimeState *state, const char *name);
int runtime_push_call(RuntimeState *state, int return_line);
int runtime_pop_call(RuntimeState *state);
void runtime_set_error(RuntimeState *state, int code, int line);
int runtime_get_error(RuntimeState *state);
```

### Responsibilities
- Variable storage and lookup
- Call stack for GOSUB/RETURN
- FOR loop context management
- Error state tracking
- File handle management

### Dependencies
- common.h

### Status
🚧 STUB (Phase 3)

---

## 8. EVAL (`eval.h/c`) - PHASE 3

### Purpose
Evaluate expression trees (AST) to numeric/string results.

### Key Functions (To Be Implemented)
```c
double eval_numeric_expr(ASTExpr *expr, RuntimeState *state);
char *eval_string_expr(ASTExpr *expr, RuntimeState *state);
int eval_condition(ASTExpr *expr, RuntimeState *state);
```

### Responsibilities
- Walk ASTExpr nodes
- Apply operators (binary and unary)
- Look up variables from RuntimeState
- Call built-in functions
- Handle type conversions
- **Caller must free string results**

### Operator Precedence (High to Low)
```
eval_logical()        ← OR (lowest)
  ↓
eval_and_expr()       ← AND
  ↓
eval_relop()          ← Comparisons (=, <, >, <=, >=, <>)
  ↓
eval_expr()           ← Addition (+, -)
  ↓
eval_term()           ← Multiplication (*, /, MOD)
  ↓
eval_power()          ← Exponentiation (^, right-associative)
  ↓
eval_factor()         ← Numbers, variables, functions (highest)
```

### Dependencies
- ast.h
- runtime.h
- builtins.h (for function calls)

### Status
🚧 STUB (Phase 3)

---

## 9. BUILTINS (`builtins.h/c`) - PHASE 4

### Purpose
Implement built-in functions (math, string, I/O).

### Function Categories

#### Math Functions
```
SQR(), SIN(), COS(), TAN(), ATN(), EXP(), LOG()
INT(), ABS(), SGN(), RND()
```

#### String Functions
```
LEN(), LEFT$(), RIGHT$(), MID$(), ASC(), CHR$()
STR$(), VAL(), INSTR(), INKEY$()
```

#### File Functions
```
EOF(), LOC(), LOF()
```

### Key Functions (To Be Implemented)
```c
double call_numeric_function(RuntimeState *state, const char *func_name,
                             ASTExpr **args, int num_args);
char *call_string_function(RuntimeState *state, const char *func_name,
                           ASTExpr **args, int num_args);
```

### Responsibilities
- Numeric and string function evaluation
- Argument evaluation and type coercion
- Mathematical computations
- String operations
- File status queries

### Dependencies
- ast.h
- runtime.h
- eval.h (for recursive expression evaluation)

### Status
🚧 STUB (Phase 4)

---

## 10. EXECUTOR (`executor.h/c`) - PHASE 4

### Purpose
Execute AST nodes (primary tree-walking interpreter).

### Key Functions (To Be Implemented)
```c
int execute_program(RuntimeState *state, Program *prog);
int execute_statement(RuntimeState *state, ASTStmt *stmt, Program *prog);
int find_program_line(Program *prog, int line_number);
```

### Statement Handlers
- **I/O:** PRINT, INPUT, LINE INPUT, GET#, PUT#
- **Assignment:** LET (scalar and array)
- **Control Flow:** IF/THEN, FOR/NEXT, GOTO, GOSUB, RETURN
- **Data:** READ, RESTORE, DATA
- **Functions:** DEF FN, DEFINT, DEFSNG, etc.
- **File Operations:** OPEN, CLOSE, SEEK
- **Error Handling:** ON ERROR GOTO, RESUME
- **Program Control:** END, STOP, RUN, CLEAR
- **Other:** REM, POKE, DIM, BREAK

### Responsibilities
- Dispatch to statement handlers based on type
- Manage program counter and branching
- Call eval.c for expressions
- Call builtins.c for functions
- Update RuntimeState
- Handle breakpoints

### Dependencies
- ast.h
- runtime.h
- eval.h
- builtins.h
- debug.h (for breakpoints)

### Status
🚧 STUB (Phase 4)

---

## 11. DEBUG (`debug.h/c`) - PHASE 4

### Purpose
Breakpoint management and interactive debugging.

### Key Types
```c
typedef struct {
    int *breakpoints;
    int num_breakpoints;
    int max_breakpoints;
    int execution_paused;
    int step_mode;
} DebugState;
```

### Key Functions (To Be Implemented)
```c
DebugState *debug_create(void);
void debug_free(DebugState *state);
void debug_print_state(DebugState *state, RuntimeState *runtime);
void debug_print_variable(RuntimeState *runtime, const char *var_name);
```

### Responsibilities
- Breakpoint insertion/removal
- Step/Continue execution control
- State inspection commands
- Variable inspection

### Dependencies
- runtime.h

### Status
🚧 STUB (Phase 4)

---

## 12. MAIN (`main.c`) - PHASE 4

### Purpose
Entry point and REPL loop.

### Responsibilities
- Coordinate all phases: lex → parse → optimize → execute
- Handle user input in interactive mode
- File loading and program management
- Error reporting

### Key Workflow
```
1. Read user input or load file
2. Call lexer → tokenize
3. Call parser → build AST
4. Call symtable → analyze
5. (Optional) Call optimizer → optimize AST
6. Call executor → run program
7. Handle errors and output results
8. Repeat (interactive mode)
```

### Dependencies
- All other modules

### Status
🚧 STUB (Phase 4)

---

## Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│  Input: BASIC source code (file or interactive)              │
└────────────────────┬────────────────────────────────────────┘
                     │
         ┌───────────▼──────────┐
         │  LEXER (lexer.c)      │
         │  Input: text          │
         │  Output: Token[]      │
         └───────────┬───────────┘
                     │
         ┌───────────▼──────────────┐
         │  PARSER (parser.c)       │
         │  Input: Token[]          │
         │  Output: AST Program     │
         └───────────┬──────────────┘
                     │
         ┌───────────▼──────────────────┐
         │  SYMTABLE (symtable.c)       │
         │  Input: AST Program          │
         │  Output: SymbolTable         │
         └───────────┬──────────────────┘
                     │
         ┌───────────▼──────────────────┐
         │  OPTIMIZER (optimizer.c)     │  (optional)
         │  Input: AST Program          │
         │  Output: Optimized AST       │
         └───────────┬──────────────────┘
                     │
     ┌───────────────▼───────────────────────┐
     │  EXECUTOR (executor.c)                 │
     │  ┌─────────────────────────────────┐  │
     │  │ Walk AST statements             │  │
     │  │ For each statement:             │  │
     │  │  1. Call eval.c for expressions │  │
     │  │  2. Call builtins.c for funcs   │  │
     │  │  3. Update runtime.c state      │  │
     │  │  4. Check debug.c breakpoints   │  │
     │  └─────────────────────────────────┘  │
     │  Input: AST, RuntimeState             │
     │  Output: Results, state changes       │
     └───────────────┬───────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│  Output: Program results, error messages, state             │
└─────────────────────────────────────────────────────────────┘
```

---

## Dependency Graph

```
common.h ──┬──→ errors.h
           ├──→ ast.h ──┬──→ parser.h
           │           ├──→ symtable.h
           │           └──→ executor.h
           │
           ├──→ lexer.h ──→ parser.h
           │
           ├──→ runtime.h
           ├──→ eval.h ──→ builtins.h
           ├──→ executor.h
           ├──→ debug.h
           └──→ main.c

No circular dependencies (proper include hierarchy)
```

---

## Implementation Phases

### Phase 1: Foundation ✅ COMPLETE
- common.h/c (utilities)
- ast.h/c (types and utilities)
- lexer.h/c (tokenization)
- errors.h/c (error handling)

### Phase 2: Parsing
- parser.h/c (token → AST)
- symtable.h/c (type analysis)
- Unit tests (test_parser.c, test_symtable.c)

### Phase 3: Runtime
- runtime.h/c (execution state)
- eval.h/c (expression evaluation)
- builtins.h/c (built-in functions)

### Phase 4: Execution
- executor.h/c (AST tree-walker)
- debug.h/c (breakpoint support)
- main.c (REPL loop)
- Integration testing

### Phase 5: Polish
- optimizer.h/c (optional AST optimization)
- Performance profiling
- Documentation updates

---

## Quick Reference: Which File?

**Need to add...**
- Error codes? → errors.h
- Memory allocation? → common.c
- New AST node type? → ast.h
- New token type? → lexer.h
- New statement type? → ast.h (StmtType enum) + executor.c
- New expression type? → ast.h (ExprType enum) + eval.c
- New built-in function? → builtins.c
- New operator? → ast.h (OpType enum) + lexer.c + eval.c
- Variable storage? → runtime.c
- Variable lookup? → runtime.c
- New keyword? → lexer.c (tokenizer) + parser.c (parsing)

---

## Testing Strategy

### Unit Tests
```bash
make test                    # Old interpreter tests
tests/unit/test_lexer.c      # Lexer unit tests (Phase 1)
tests/unit/test_parser.c     # Parser unit tests (Phase 2)
tests/unit/test_symtable.c   # Symbol table tests (Phase 2)
tests/unit/test_eval.c       # Evaluator tests (Phase 3)
```

### Integration Tests
```bash
tests/basic_tests/*.bas      # All 24 existing tests
                             # Run against both old and new interpreter
                             # Compare output byte-for-byte
```

### Regression Testing
```bash
# After each phase, run full test suite
make clean && make ast-build && ./build/bin/basic-trs80-ast < tests/basic_tests/01_helloworld.bas
```

---

## Summary

The AST architecture provides clear component boundaries with well-defined interfaces. Each component can be:
- ✅ Tested independently
- ✅ Modified without affecting others
- ✅ Reused for different BASIC dialects
- ✅ Understood by new developers
- ✅ Extended with new features

**Phase 1 complete.** Ready for Phase 2 (Parser implementation).
