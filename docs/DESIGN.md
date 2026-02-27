# Basic++ Design Document

## Architecture Overview

Basic++ is a line-number-free, object-oriented BASIC interpreter with a focus on clean procedural programming in Phase 1, establishing foundation for OOP in Phases 2-3.

### High-Level Architecture

```
┌─────────────────────────────────────────┐
│  VS Code Extension (TypeScript)         │
│  - Syntax highlighting                  │
│  - Variable inspector                   │
│  - Debugger UI                          │
└────────────┬────────────────────────────┘
             │
        ┌────┴──────────────────────────┐
        │                               │
   ┌────▼────────────┐      ┌──────────▼─────────┐
   │ basic++ CLI     │      │ basic++ --debug    │
   │ (normal run)    │      │ (debug mode)       │
   └────────────────┘      └────────────────────┘
        │                               │
   ┌────▼───────────────────────────────▼────┐
   │  Basic++ Interpreter (C)                │
   │  ┌──────────────────────────────────┐   │
   │  │ Lexer       → Parser → AST       │   │
   │  │                                  │   │
   │  │ Runtime (variables, scope)       │   │
   │  │ Executor (statement execution)   │   │
   │  │ Builtins (math, string, I/O)     │   │
   │  └──────────────────────────────────┘   │
   └─────────────────────────────────────────┘
```

---

## Core Components

### 1. Lexer (src/lexer.c)

**Purpose:** Tokenization

**Key Changes from TRS-80:**
- REMOVE: Line number parsing as statement prefix
- REMOVE: GOTO, GOSUB tokens (obsolete)
- ADD: PROCEDURE, END, RETURN tokens
- ADD: Better identifier handling (no line-number prefix)

**Output:** Stream of tokens (no line numbers)

### 2. Parser (src/parser.c)

**Purpose:** Build Abstract Syntax Tree (AST)

**Key Changes from TRS-80:**
- REMOVE: Line-number-based statement lookup
- REMOVE: GOTO/GOSUB parsing
- ADD: Procedure definition parsing
- ADD: Scope-aware statement parsing
- ADD: Nested block support (IF/FOR/WHILE)

**AST Nodes:**
```c
typedef struct {
    char *name;
    List *parameters;      // param names
    ASTStmt *body;        // procedure body
} ProcedureDefNode;

typedef struct {
    char *name;
    ASTExpr *arguments;   // call args
} ProcedureCallNode;
```

### 3. AST (src/ast.c)

**Purpose:** Abstract Syntax Tree representation

**Key Change:** Add procedure and scope metadata

```c
enum ASTStatementType {
    STMT_PROCEDURE_DEF,
    STMT_PROCEDURE_CALL,
    STMT_RETURN,
    // ... existing: IF, FOR, WHILE, PRINT, etc.
};

typedef struct ASTStmt {
    enum ASTStatementType type;
    
    // For PROCEDURE_DEF:
    char *proc_name;
    List *params;           // param names
    ASTStmt *proc_body;
    int proc_scope_id;      // scope namespace
    
    // ... other statement fields
} ASTStmt;
```

### 4. Runtime (src/runtime.c)

**Purpose:** Variable storage, scoping, memory management

**Key Changes:**
- REMOVE: Global-only symbol table assumption
- ADD: Scope stack
- ADD: Stack frame for each procedure call
- ADD: Local variable lookup chain

**Scope Structure:**
```c
typedef struct Scope {
    int scope_id;
    SymbolTable *local_vars;
    struct Scope *parent;    // for variable lookup chain
} Scope;

typedef struct RuntimeState {
    Stack *scope_stack;      // Current scope chain
    SymbolTable *globals;    // Global variables
    // ... other state
} RuntimeState;
```

**Variable Lookup:**
```
When accessing variable X:
1. Search current scope locals
2. Search parent scope (if any)
3. Search globals
4. Error: undefined variable
```

### 5. Executor (src/executor.c)

**Purpose:** Execute statements

**Key Changes:**
- REMOVE: GOTO target resolution
- ADD: Procedure call frame creation
- ADD: Scope push/pop on procedure entry/exit
- ADD: Proper return value handling

**Execution Flow:**
```
1. Push new scope for procedure
2. Create local variables on scope
3. Execute statement block
4. On RETURN: pop scope, return value
5. Pop scope on END PROCEDURE
```

### 6. Builtins (src/builtins.c)

**Purpose:** Math, string, I/O functions

**No major changes,** but ensure functions can access current scope for variable updates.

---

## Memory Management Strategy

### Phase 1 (Simple)

**Variables:**
- Allocated in scope structure
- Freed when scope exits (automatic)
- Reference counting for strings

**Procedures:**
- AST stored in program structure
- Destroyed on program exit

**Future (Phase 2+):**
- Garbage collection for objects
- Custom memory pools for performance

---

## Scoping Implementation

### Global Scope
```
┌─────────────────────────┐
│ Global Variables        │
│ - X = 5                 │
│ - Name$ = "Global"      │
└─────────────────────────┘
```

### Procedure Scope
```
┌─────────────────────────┐
│ Procedure A             │
│ Local Variables:        │
│ - LocalA = 10          │
│ - Data$ = "A"          │
│ Parent: Global         │
└─────────────────────────┘
         │ lookup chain
         ▼
    ┌──────────────┐
    │ Global Vars  │
    └──────────────┘
```

### Nested Scope (Procedure calling Procedure)
```
┌─────────────────────────┐
│ Procedure B             │
│ Local Variables:        │
│ - LocalB = 20          │
│ Parent: Global         │
└─────────────────────────┘
         │ lookup chain
         ▼
    ┌──────────────┐
    │ Global Vars  │
    └──────────────┘
```

---

## Control Flow

### Procedure Calls

```
PROCEDURE Add(X, Y)
  RETURN X + Y
END PROCEDURE

LET Result = Add(3, 5)
```

**Execution:**
1. Parse `Add(3, 5)` → ProcedureCallNode
2. Executor sees PROCEDURE_CALL
3. Create new scope, assign parameters: X=3, Y=5
4. Execute procedure body
5. RETURN pops scope, returns value

### No Jumps

- GOTO eliminated
- GOSUB eliminated
- All control flow via structured blocks:
  - IF/THEN/ELSE/ENDIF
  - FOR/NEXT
  - WHILE/WEND
  - DO/LOOP
  - PROCEDURE/END PROCEDURE

### Block Statements

Multi-statement blocks are sequences of statements:

```c
typedef struct {
    ASTStmt **statements;
    int count;
} BlockNode;
```

OR (simpler): Chain statements via `next` pointer:
```c
stmt->next = next_stmt;
```

---

## Parsing Strategy

### Recursive Descent Parser

1. **parse_program()** → Parse all top-level definitions/statements
2. **parse_procedure()** → Parse PROCEDURE...END PROCEDURE
3. **parse_statement()** → Parse one statement- **parse_if()**, **parse_for()**, etc. → Specialized statement parsing
4. **parse_expression()** → Parse expressions (unchanged from TRS-80)

### Key Parser Changes

```c
// OLD TRS-80 (line-number based):
ProgramLine *parse_line() {
    int line_num = expect_number();  // Mandatory line number
    ASTStmt *stmt = parse_statement();
    return create_program_line(line_num, stmt);
}

// NEW Basic++ (line-number free):
ASTStmt *parse_statement() {
    Token *tok = current_token();
    
    switch(tok->type) {
        case TOK_PROCEDURE:
            return parse_procedure();
        case TOK_IF:
            return parse_if();
        // ... other statements
    }
}
```

---

## Test Strategy

### Unit Tests (Phase 1)

```
tests/
├── test_lexer/
│   ├── test_tokens_procedures.bas
│   ├── test_identifiers.bas
│   └── expected_output/
├── test_parser/
│   ├── test_procedure_parsing.bas
│   ├── test_nested_blocks.bas
│   └── expected_output/
├── test_scoping/
│   ├── test_local_variables.bas
│   ├── test_parameter_passing.bas
│   ├── test_nested_procedures.bas
│   └── expected_output/
└── test_execution/
    ├── test_procedure_calls.bas
    ├── test_return_values.bas
    └── expected_output/
```

### Integration Tests

- Full programs similar to dartmouth.bas (no line numbers)
- Debug mode variable inspection
- Extension syntax highlighting

---

## CLI Interface

### Normal Execution
```bash
$ ./basic++ program.bas
(Output from program)

$ ./basic++ --version
Basic++ v0.1 (Phase 1)

$ ./basic++ --help
Usage: basic++ [options] [program.bas]
  --lsp         Run as LSP server
  --debug       Run with debug output
  --version     Show version
```

### Debug Mode (for extension)

```bash
$ ./basic++ --debug program.bas
{"cmd": "start", "program": "program.bas"}
[pause at first breakpoint or on demand]
{"cmd": "variables", "vars": [...]}
```

---

## Performance Considerations (Phase 4+)

- Current focus: **Correctness** over speed
- Compiler (Phase 4) will optimize to LLVM IR
- Interpreter does NOT need optimization yet

---

## Compatibility Notes

### vs. TRS-80 BASIC
- **Incompatible:** Line numbers not supported
- **Incompatible:** GOTO/GOSUB not supported
- **Incompatible:** ON...GOTO not supported
- **Compatible:** All math/string functions
- **Compatible:** FOR/WHILE/IF/PRINT/INPUT

### vs. Traditional BASIC
- **Similar:** Procedure concept (like SUB in some dialects)
- **Different:** Line-number-free (like modern scripting)
- **Different:** Structured scoping (like procedural languages)

---

## Build System

**Makefile targets:**
- `make` - Build interpreter
- `make debug` - Build with debug symbols
- `make test` - Run test suite
- `make clean` - Remove build artifacts
- `cd extension && npm install && npm run compile` - Build VS Code extension

---

## Timeline (Phase 1)

| Week | Milestone |
|------|-----------|
| 1 | Lexer modifications, test infrastructure |
| 2 | Parser rewrite (procedure/scope support) |
| 3 | Runtime scope stack, executor updates |
| 4 | Testing, bug fixes, documentation |
| 5-6 | Integration with extension, final polish |
