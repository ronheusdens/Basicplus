# Phase 1 Session 2 - Roadmap

**Estimated Duration:** 2-3 hours  
**Sessions Completed:** 1/4  
**Target Completion:** Within 2-3 more sessions

## Session 2 Goals

### Primary Deliverable
**Remove line-number requirement from parser + implement scope stack**

### Success Criteria
1. ✅ `make` compiles with zero errors
2. ✅ Parser accepts `PROCEDURE name() ... END PROCEDURE` without line numbers
3. ✅ Scope stack works: local variables don't affect globals
4. ✅ One test program demonstrates procedure call with local vars

---

## Detailed Work Breakdown

### Task 1: Scope Stack Implementation (45 min)

**File:** `src/runtime.h` and `src/runtime.c`

#### Step 1.1: Define Scope Structures (runtime.h)
```c
typedef struct Scope {
    int scope_id;                    // Unique identifier
    SymbolTable *local_vars;         // Local variables
    struct Scope *parent;            // For lookup chain
} Scope;

typedef struct {
    Scope **stack;                   // Array of scopes
    int depth;                       // Current depth
    int capacity;                    // Allocated size
    int next_scope_id;              // Counter for scope IDs
} ScopeStack;
```

#### Step 1.2: Add Scope Functions (runtime.h declarations)
- `scope_create(Scope *parent)` → Scope*
- `scope_free(Scope *scope)` → void
- `scope_stack_create()` → ScopeStack*
- `scope_stack_free(ScopeStack *stack)` → void
- `scope_push(ScopeStack *stack, Scope *scope)` → void
- `scope_pop(ScopeStack *stack)` → void
- `scope_current(ScopeStack *stack)` → Scope*
- `scope_lookup_variable(Scope *scope, const char *name)` → Variable or NULL

#### Step 1.3: Implement in runtime.c
- Use dynamic array pattern (seen in symtable.c)
- Parent pointer enables variable lookup chain
- Clean memory management with proper freeing

#### Step 1.4: Integrate with RuntimeState
- Add `ScopeStack *scope_stack` to struct RuntimeState
- Initialize in `runtime_create()`
- Free in `runtime_free()`

**Expected Impact:** Runtime gains scope isolation capability but doesn't use it yet (parser still requires line numbers)

---

### Task 2: Parser Line-Number Removal (60 min)

**File:** `src/parser.c`

#### Step 2.1: Understand Current Structure
- `parse_program()` → Calls `parse_line()` in loop
- `parse_line()` → Expects `TOK_NUMBER` or fails
- Need to change: accept any statement or PROCEDURE

#### Step 2.2: Modify parse_program()
```c
// OLD: Requires line number before each statement
while (...) {
    Token *tok = current_token();
    ProgramLine *line = parse_line(parser);  // line = expects number + statement
}

// NEW: Accept statements and procedures without numbers
while (...) {
    Token *tok = current_token();
    
    // Check if it's a PROCEDURE definition
    if (tok->type == TOK_PROCEDURE) {
        ASTStmt *proc_def = parse_procedure_def(parser);
        // Store procedure in program somehow (new structure)
        continue;
    }
    
    // Check if it's a statement
    if (is_statement_token(tok)) {
        ASTStmt *stmt = parse_statement(parser);
        // Store statement (maybe as Program
Line with line_num=0?)
        continue;
    }
    
    // Check for old-style line number
    if (tok->type == TOK_NUMBER) {
        int line_num = ...;
        ASTStmt *stmt = parse_statement(parser);
        // Store as ProgramLine with line_num
        continue;
    }
    
    advance(parser);  // Skip unknown token
}
```

#### Step 2.3: Create parse_procedure_def()
```c
ASTStmt *parse_procedure_def(Parser *parser)
{
    // Expect: PROCEDURE Name ( param_list ) ... END PROCEDURE
    expect(parser, TOK_PROCEDURE);
    char *name = current_token()->value;  // Procedure name
    expect(parser, TOK_IDENTIFIER);
    
    expect(parser, TOK_LPAREN);
    ASTParameterList *params = parse_parameter_list(parser);
    expect(parser, TOK_RPAREN);
    
    // Parse body (statements until END PROCEDURE)
    ASTStmt *body = parse_statement_block(parser, TOK_END);
    
    expect(parser, TOK_END);
    expect(parser, TOK_PROCEDURE);
    
    ASTStmt *proc = ast_stmt_create(STMT_PROCEDURE_DEF);
    proc->var_name = xstrdup(name);
    proc->parameters = params;
    proc->body = body;
    return proc;
}
```

#### Step 2.4: Create parse_parameter_list()
```c
ASTParameterList *parse_parameter_list(Parser *parser)
{
    ASTParameterList *list = ast_parameter_list_create();
    
    while (current_token()->type != TOK_RPAREN) {
        char *param_name = current_token()->value;
        expect(parser, TOK_IDENTIFIER);
        
        VarType type = VAR_DOUBLE;  // default
        if (match(parser, TOK_AS)) {
            // Parse type keyword (INTEGER, STRING, etc.)
            Token *type_tok = current_token();
            type = parse_type_keyword(type_tok);
            advance(parser);
        }
        
        ast_parameter_list_add(list, param_name, type);
        
        if (!match(parser, TOK_COMMA))
            break;
    }
    
    return list;
}
```

#### Step 2.5: Update is_statement_token()
- Add check for all statement keywords (PRINT, IF, FOR, etc.)
- Used to distinguish statements from procedures

**Expected Impact:** Parser no longer requires line numbers; accepts mixed mode (procedures + statements)

---

### Task 3: Executor Scope Integration (30 min)

**File:** `src/executor.c`

#### Step 3.1: Handle STMT_PROCEDURE_DEF
```c
case STMT_PROCEDURE_DEF:
{
    // Store procedure in global registry (placeholder for now)
    // Don't execute, just register
    // (Full implementation in Phase 1.3)
    return 0;
}
```

#### Step 3.2: Skip Procedure Calls for Now
- Add handling for STMT_PROCEDURE_CALL that just returns 0
- Will implement in Phase 1.3

#### Step 3.3: Add Scope Push/Pop Hooks
- (Placeholder for when procedure calls work)

**Expected Impact:** Executor accepts procedure definitions without crashing

---

### Task 4: Basic Testing & Validation (15 min)

#### Test 1: Parser Accepts Procedures
Create `tests/test_procedure_syntax.bas`:
```basic
PROCEDURE Greet(name$)
    PRINT "Hello, "; name$
END PROCEDURE

PRINT "Test starting"
```

Run: `./build/bin/basicpp tests/test_procedure_syntax.bas`  
Expected: No parse errors, clean exit

#### Test 2: Line Numbers Still Work
Create `tests/test_backward_compat.bas`:
```basic
10 PRINT "Line with number"
20 LET X = 5
30 PRINT X
```

Run: `./build/bin/basicpp tests/test_backward_compat.bas`  
Expected: Execute normally, print 5

#### Test 3: Scope Stack Initialized
Add debug output to runtime_create():
```c
// In runtime_create():
printf("DEBUG: Initializing scope stack\n");
state->scope_stack = scope_stack_create();
```

Run: `./build/bin/basicpp tests/test_backward_compat.bas`  
Expected: See "DEBUG: Initializing scope stack" printed

---

## Implementation Checklist

### Scope Stack (runtime.h/c)
- [ ] Define Scope struct
- [ ] Define ScopeStack struct
- [ ] Implement scope_create()
- [ ] Implement scope_free()
- [ ] Implement scope_stack_create()
- [ ] Implement scope_stack_free()
- [ ] Implement scope_push()
- [ ] Implement scope_pop()
- [ ] Implement scope_current()
- [ ] Add scope_stack to RuntimeState
- [ ] Initialize/free in runtime_create/free()
- [ ] Compile test: `make` succeeds

###Parser Modifications (parser.c)
- [ ] Create is_statement_token(Token*) helper
- [ ] Modify parse_program() structure
- [ ] Create parse_procedure_def()
- [ ] Create parse_parameter_list()
- [ ] Create parse_type_keyword() helper
- [ ] Create parse_statement_block() if needed
- [ ] Update parser error handling
- [ ] Compile test: `make` succeeds

### Executor Updates (executor.c)
- [ ] Handle STMT_PROCEDURE_DEF case (stub)
- [ ] Handle STMT_PROCEDURE_CALL case (stub)
- [ ] Compile test: `make` succeeds

### Testing
- [ ] Create test file 1: procedure syntax
- [ ] Create test file 2: backward compatibility
- [ ] Create test file 3: scope debug output
- [ ] Verify all tests run without errors

---

## Known Challenges

1. **Program Structure Change**: Program currently stores only `ProgramLine` (with line numbers). May need:
   - Add `Procedure *` array to Program struct, OR
   - Change ProgramLine to allow line_num=0 for non-line-numbered statements
   
2. **Backward Compatibility**: Must still support line numbers for TRS-80 programs

3. **Statement vs. Procedure Distinction**: Parser must recognize PROCEDURE keyword after newline

---

## Success Metrics

After Session 2:
- ✅ `make` compiles without errors
- ✅ Parser accepts `PROCEDURE...END PROCEDURE` syntax
- ✅ Parser still accepts line numbers (backward compat)
- ✅ Scope stack API ready (tested at runtime_create time)
- ✅ 3 basic test programs demonstrate mixed syntax
- ✅ No regression in existing functionality

---

## Session 2 Setup

Before starting, ensure:
1. Current work committed to git
2. `docs/PHASE1_SESSION1_SUMMARY.md` reviewed
3. `docs/DESIGN.md` section on scoping re-read
4. `build/bin/basicpp` current state verified

Start with scope stack (most self-contained), then parser (most complex), then executor (simplest).

Estimated completion: 2-3 hours
