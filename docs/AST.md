# AST.md

## Purpose

An **Abstract Syntax Tree (AST)** is a structured, tree-shaped representation of source code after parsing. It captures the *meaningful* structure of a program (expressions, statements, declarations), while skipping most surface details (whitespace, many parentheses, exact formatting).

An AST is built to:
- Separate parsing from execution or compilation.
- Enable semantic analysis (names, scopes, types).
- Enable transformations (optimizations, refactors, codegen).
- Improve error reporting and tooling.

---

## Core Concepts

### Concrete Syntax Tree vs Abstract Syntax Tree
- **Concrete Syntax Tree (CST)** mirrors grammar rules closely and includes syntactic artifacts.
- **AST** keeps only semantic structure.

Example:

Source:
```c
a = (b + 3) * c;
```

AST (conceptually):
- Assign
  - Identifier(a)
  - Multiply
    - Add
      - Identifier(b)
      - Int(3)
    - Identifier(c)

Parentheses are not stored because precedence is encoded by the tree shape.

---

## Typical AST Node Model

### Node essentials
Most AST nodes contain:
- **kind**: what node type it is (BinaryExpr, IfStmt, FunctionDecl).
- **children**: sub-nodes.
- **payload**: values like operator type, identifier name, literal value.
- **source span**: start and end positions for diagnostics.
- **optional semantic info** (added later): resolved symbol, inferred type, constant value.

### Example node representation (conceptual)
A common pattern is a tagged union:

- `NodeKind` enum identifies the active variant.
- `Node` contains a `kind`, `span`, and a `union` of per-kind structs.

Key design choices:
- Keep node structs small.
- Avoid deep copying large strings (intern identifiers).
- Store source ranges everywhere that can generate an error.

---

## Building the AST

### Pipeline overview
1. **Lexing**: source text -> tokens.
2. **Parsing**: tokens -> AST.
3. **Semantic passes**: annotate AST (scopes, types, symbols).
4. **Lowering or codegen**: AST -> IR/bytecode/machine code, or interpretation.

---

## Step 1: Lexing Into Tokens

The lexer produces tokens like:
- Keywords: `if`, `while`, `return`
- Identifiers: `myVar`
- Literals: `123`, `"hi"`
- Operators: `+`, `*`, `=`
- Punctuation: `(` `)` `{` `}` `;`

Each token usually carries:
- type
- lexeme (or interned reference)
- source position and length

---

## Step 2: Parsing Strategy

Two common strategies:

### Recursive descent (common in small languages)
- Each grammar rule is a function.
- Functions build and return AST nodes.

Pros:
- Simple and readable.
- Good error recovery can be added.

Cons:
- Manual precedence handling for expressions unless you use a helper.

### Pratt parser (expression parsing)
- Designed for expressions with precedence and associativity.
- Clean handling of operators.

Pros:
- Excellent for expression-heavy languages.

Cons:
- Slightly less intuitive at first.

---

## Step 3: Expression Parsing and Node Construction

### Operator precedence in the AST
Precedence is reflected by how nodes nest.

Example:
```txt
1 + 2 * 3
```

AST:
- Add(1, Multiply(2, 3))

### Parser responsibilities when building nodes
- Decide node kind.
- Attach child pointers.
- Attach source span.
- Apply desugaring if desired (optional).

---

## Step 4: Statement and Block Nodes

Statements are usually a higher-level layer than expressions:
- ExpressionStmt
- VarDecl
- IfStmt
- WhileStmt
- ReturnStmt
- BlockStmt (list of statements)

Blocks typically contain:
- list of statement nodes
- source span for `{ ... }`

---

## AST Memory Management

### Allocation strategies
ASTs create lots of small nodes. Common approaches:

1. **Arena allocation**
   - Allocate nodes from a bump allocator.
   - Free everything at once when compilation ends.

   Pros:
   - Very fast.
   - Simple lifetime management.

   Cons:
   - You cannot free individual nodes.
   - Editing tools may need additional strategies.

2. **Reference counting**
   - Less common for AST nodes, more overhead.

3. **Manual malloc/free**
   - Works but becomes error-prone for large trees.

### Ownership rules
Define these early:
- The AST owns its nodes.
- Strings are interned and owned by an interner, not by nodes.
- Lists (children arrays) are either in arena or owned by the node.

---

## Source Spans and Diagnostics

### Why spans matter
Without spans, error messages become vague. With spans, you can:
- underline the exact region in source
- show contextual snippets
- attach notes and related locations

### Span strategy
- Every node stores `Span { start, end }` in token offsets or (line, column).
- Error reporting uses spans from the nearest relevant node.

---

## Maintaining the AST After Parsing

Once built, the AST becomes the primary structure passed through later phases. Maintenance typically includes:

### 1) Adding semantic annotations
You do not rebuild the whole tree. You attach extra information:

- **Symbol resolution**
  - Each `Identifier` node gets a link to its `Symbol` (variable/function).
  - Declarations create symbols in a scope.

- **Scope trees**
  - BlockStmt creates a new scope.
  - FunctionDecl creates parameter scope and body scope.

- **Type information**
  - Expression nodes store inferred types or declared types.
  - Cast nodes store both source and target type.

### 2) Building auxiliary tables
Instead of bloating every node, store lookups elsewhere:
- symbol table (scopes and symbols)
- type table (interned types)
- constant table
- function table

The AST keeps references (IDs or pointers) into these tables.

---

## Multi-Pass Compilation and AST Invariants

### Why multiple passes
Many languages allow forward references or require global knowledge:
- functions used before declared
- mutually recursive types
- imports and modules

Typical pass order:
1. Parse AST
2. Collect declarations (top-level symbols)
3. Resolve names
4. Type check
5. Evaluate constants
6. Lowering/codegen

### Invariants to enforce
Define what must always be true:
- All child pointers are valid.
- Node kind matches payload.
- Spans are within source bounds.
- After name resolution: all identifiers in valid contexts have symbols.
- After type checking: all expression nodes have a type.

Use asserts in debug builds to keep the tree trustworthy.

---

## Error Recovery While Building the AST

### Why recovery matters
Stopping at the first error is painful for users. Good recovery means you can report multiple errors per run.

Common recovery tactics:
- Synchronize at known boundaries: `;`, `}`, newline, `end`.
- Create **Error nodes**:
  - `ErrorExpr`, `ErrorStmt`
  - Carry span and optionally a message
  - Allow later passes to continue without crashing

Example:
- If an expression fails, return `ErrorExpr` so the statement can still be constructed.

---

## Expanding the AST Over Time

As your language grows, your AST must evolve without turning into a mess.

### Principle: separate syntax from semantics
- Keep syntactic shape nodes stable.
- Store semantic info as optional fields or external tables.

### Step-by-step approach to expansion

#### Step 1: Add a new grammar feature
Example feature: `for` loops.

#### Step 2: Decide whether to represent directly or desugar
Two options:

**A. Direct node**
- Add `ForStmt { init, cond, step, body }`

Pros:
- Clear for tooling.
- Mirrors the source.

Cons:
- Interpreter/codegen must handle it.

**B. Desugar to existing nodes**
- Parse `for` then convert into:
  - init statement
  - while statement with body extended by step

Pros:
- Less backend code.

Cons:
- Harder to preserve original structure for tooling.

A hybrid is common:
- Keep `ForStmt` in AST for analysis.
- Lower it later into `WhileStmt` in a lowering pass.

#### Step 3: Update visitors and passes
Every new node kind touches:
- pretty printer
- validator
- name resolver
- type checker
- interpreter or code generator

#### Step 4: Ensure backwards compatibility
If the AST is used by tooling or tests:
- keep stable serialization or provide migrations
- add versioning to AST dumps

---

## Common Patterns for Working With ASTs

### Visitor pattern
Implement operations over many node kinds:
- printing
- interpreting
- type checking
- constant folding

Approaches:
- switch on `kind` with functions per node type
- function pointers table keyed by `NodeKind`

### Pattern matching via switches
In C, you often use:
- a `switch(kind)`
- handle each node type explicitly
- default to error in debug builds

This is verbose but reliable.

---

## AST Validation and Testing

### Validation
Write a validator that walks the tree and checks invariants:
- correct child relationships
- no null children where required
- spans monotonic and in range
- no unresolved identifiers after resolution phase

Run validation:
- after parsing
- after each semantic pass
- after transformations

### Testing strategies
- golden tests: parse source and compare AST dump
- fuzz tests: random token streams, ensure no crashes
- round-trip: pretty print AST and reparse, compare structure where appropriate

---

## Incremental AST Updates (Editors and IDEs)

If you need fast updates during editing:
- parse into a CST to preserve exact structure, then derive AST
- use incremental parsing libraries (tree-sitter style)
- keep stable node IDs so tooling can track nodes across edits
- store "green tree" (immutable) plus "red tree" (annotated) patterns if needed

For most compilers and interpreters, a full rebuild per file is sufficient.

---

## Practical Checklist

When implementing an AST system:

- [ ] Define node kinds and their fields.
- [ ] Decide on memory strategy (arena recommended).
- [ ] Store spans in all nodes.
- [ ] Build a parser that constructs nodes consistently.
- [ ] Add error nodes and recovery to keep parsing going.
- [ ] Implement visitors or a clean dispatch mechanism.
- [ ] Create symbol tables and name resolution pass.
- [ ] Add type checking pass if applicable.
- [ ] Validate invariants after each major phase.
- [ ] Plan how new syntax will be represented or lowered.

---

## Glossary

- **Token**: smallest meaningful unit from lexing.
- **Parser**: turns tokens into structured representation.
- **AST**: abstract representation of program structure.
- **Span**: source location interval for diagnostics.
- **Symbol**: a declared entity (variable/function/type).
- **Scope**: region where symbols are visible.
- **Lowering**: converting AST into simpler IR for execution/codegen.
- **Desugaring**: rewriting new syntax into older core constructs.
