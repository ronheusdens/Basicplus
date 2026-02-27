# Basic++ Phase 1 Implementation Progress

**Start Date:** February 27, 2026  
**Target Completion:** April 3-10, 2026 (5-6 weeks)

## Milestone Breakdown

### Week 1: Foundation (Line Numbers → Procedures)

#### Day 1-2: Scope Stack Implementation
- [x] Review current runtime structure
- [ ] Design Scope stack (Scope, ScopeStack types)
- [ ] Implement scope_create(), scope_free()
- [ ] Implement scope_push(), scope_pop()
- [ ] Implement scope-aware variable lookup
- [ ] Add scope_enter_procedure() and scope_exit_procedure()

#### Day 3: Lexer Updates - Add PROCEDURE Keyword
- [ ] Add TOK_PROCEDURE to lexer.h token types
- [ ] Add "PROCEDURE" to keyword table in lexer.c
- [ ] Test keyword recognition

#### Day 4-5: Parser Refactoring - Remove Line Numbers
- [ ] Analyze current parse_line() and parse_program()
- [ ] Create new parse_statement_at_top_level()
- [ ] Modify parse_program() to handle procedures + statements
- [ ] Remove mandatory TOK_NUMBER requirement
- [ ] Handle both old (line numbers) and new (procedure) syntax

### Week 2: Procedure Support

#### Day 6-7: PROCEDURE Parsing
- [ ] Create parse_procedure_def() function
- [ ] Add PROCEDURE AST node (STMT_PROCEDURE_DEF)
- [ ] Parse procedure signature: PROCEDURE Name(param1, param2, ...)
- [ ] Parse procedure body (statements within block)
- [ ] Parse END PROCEDURE

#### Day 8: Parameter Support
- [ ] Add parameter list to AST
- [ ] Parse parameter names and types (with AS keyword)
- [ ] Symbol resolution for parameters

#### Day 9: RETURN Statement
- [ ] UPDATE: RETURN already exists in lexer/AST
- [ ] Add RETURN value parsing
- [ ] Executor support for RETURN in procedures

### Week 3: Integration & Testing

#### Day 10-11: Executor Integration
- [ ] Create ExecutionFrame for procedure calls
- [ ] Scope push/pop on procedure entry/exit
- [ ] Parameter binding in new scope
- [ ] RETURN statement execution
- [ ] Procedure call resolution

#### Day 12: Control Flow Verification
- [ ] Test IF/FOR/WHILE with procedure-local variables
- [ ] Test nested procedure calls
- [ ] Test variable shadowing (local vs global)

#### Day 13-14: Test Suite & Examples
- [ ] Create test programs demonstrating features
- [ ] Build comprehensive test suite (15+ test programs)
- [ ] Validation of lexer, parser, executor changes

### Week 4: Compilation & Deployment

#### Day 15: Clean Compile
- [ ] Verify zero compilation warnings
- [ ] Build binary successfully
- [ ] Document any breaking changes from TRS-80

#### Day 16: Phase 1 Documentation
- [ ] Update GRAMMAR.md with implementation notes
- [ ] Create PROCEDURES.md guide for feature usage
- [ ] Finalize API documentation

## Current Status

- ✅ Phase 0 Complete: Repository initialized, extension framework, grammar spec
- ✅ Build system working: `make` produces clean build
- ⏳ **Phase 1 Starting: Scope stack + Procedure support**

## Key Design Decisions

1. **Scope Model:** Stack-based with lookup chain (local → parent → global)
2. **Line Numbers:** Syntax remains compatible but not required
3. **Procedures:** Top-level definitions with local variables
4. **Parameters:** Positional, by-value for scalars, by-reference for arrays

## Files to Modify

- `src/lexer.h` - Add TOK_PROCEDURE
- `src/lexer.c` - Add PROCEDURE keyword
- `src/ast.h` - Add PROCEDURE AST nodes
- `src/parser.h` - Add procedure parsing declarations
- `src/parser.c` - Add parse_procedure_def(), modify parse_program()
- `src/runtime.h` - Add scope stack API
- `src/runtime.c` - Implement scope stack
- `src/executor.h` - Add procedure execution declarations
- `src/executor.c` - Implement procedure calls and scope management
- `src/main.c` - Update for new syntax (may need adjustments)

## Testing Strategy

1. **Unit tests** - Each component (scope, parse_procedure, executor)
2. **Integration tests** - Procedures with local variables, nested calls
3. **Regression tests** - Verify existing TRS-80 features still work
4. **Example programs** - Binary search, factorial, utilities

