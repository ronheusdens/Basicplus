## AST, Lexer, and Parser Design

This document covers the lexer, parser, AST node structure, and statement/expression flow in the interpreter.

### Lexer
- Tokenizes BASIC source code into tokens.
- Handles keywords, identifiers, numbers, strings, operators.

### Parser
- Builds an abstract syntax tree (AST) from tokens.
- Supports line-numbered statements, expressions, and control flow.

### AST Nodes
- Represent statements, expressions, and program structure.
- Created via ast_expr_create() and ast_stmt_create().

### Statement/Expression Flow
- Parser produces AST nodes for each statement.
- Executor walks the AST to execute statements.

### Reference
- See AST.md, AST_IMPLEMENTATION_ANALYSIS.md for deep dives.