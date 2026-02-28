# Phase 2 Architecture Overview

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      BasicPP Program                         │
│                  (line-number-free BASIC)                    │
└────────────────────┬────────────────────────────────────────┘
                     │
      ┌──────────────┼──────────────┐
      │              │              │
      ▼              ▼              ▼
  ┌────────┐   ┌──────────┐   ┌─────────────┐
  │ GLOBAL │   │PROCEDURE │   │   CLASS     │
  │ CODE   │   │   DEF    │   │  DEF        │
  │        │   │          │   │             │
  │PRINT X │   │PROCEDURE │   │CLASS Point()│
  │X = 5   │   │ Calc()   │   │  PROCEDURE  │
  │        │   │END PROC  │   │  Distance() │
  │        │   │          │   │END CLASS    │
  └────────┘   └──────────┘   └─────────────┘
      │              │              │
      └──────────────┼──────────────┘
                     │
                    ▼
        ┌────────────────────────┐
        │      EXECUTOR          │
        │  (Runs Program)        │
        │                        │
        │ • Procedures→Registry  │
        │ • NEW→Instance Mgr     │
        │ • obj.method→Dispatch  │
        └────────────────────────┘
```

## Lexer → Parser → AST → Runtime Flow

```
Source Code
     │
     ▼
┌──────────────┐
│    LEXER     │  Tokenizes: CLASS, NEW, DOT, PROCEDURE, etc.
│              │─── CLASS, IDENTIFIER(Point), LPAREN, ...
└──────────────┘
     │
     ▼
┌──────────────┐  Builds AST
│   PARSER     │  CLASS Point → STMT_CLASS_DEF
│              │  NEW Point(...) → EXPR_NEW
│              │  obj.Method() → EXPR_MEMBER_ACCESS
└──────────────┘
     │
     ▼
┌──────────────┐
│     AST      │  Abstract Syntax Trees
│              │  ┌─ STMT_CLASS_DEF node
│              │  │  ├─ parameters: [X, Y, ...]
│              │  │  └─ body: PROCEDURE nodes
│              │  │
│              │  ├─ EXPR_NEW node
│              │  │  ├─ class_name: "Point"
│              │  │  └─ arguments: [expr1, expr2]
│              │  │
│              │  └─ EXPR_MEMBER_ACCESS node
│              │     ├─ object: EXPR_VAR (P1)
│              │     └─ member: "Distance"
└──────────────┘
     │
     ▼
┌──────────────┐
│   EXECUTOR   │  
│   + EVAL     │
└───┬──────────┘
    │
    ├─ CLASS Point → ClassRegistry.add("Point", ...)
    │
    ├─ P = NEW Point(3,4) → ObjectInstance created
    │                      └─ instance.id = 1, class_name="Point"
    │                      └─ instance.scope = new Scope()
    │
    ├─ P.Distance(6,8) → EXPR_MEMBER_ACCESS evaluated
    │                    └─ object=P(id:1), member=Distance
    │                    └─ Lookup method in class def
    │                    └─ Execute with object context
    │
    └─ Variables, Methods, Results
```

## Runtime Data Structures

### Class Registry
```
ClassRegistry
  ├─ class[0]: ClassDef
  │   ├─ name: "Point"
  │   ├─ parameters: [X, Y]
  │   └─ body: STMT_PROCEDURE_DEF* (methods)
  │
  ├─ class[1]: ClassDef
  │   ├─ name: "Rectangle"
  │   ├─ parameters: [Width, Height]
  │   └─ body: STMT_PROCEDURE_DEF* (methods)
  │
  └─ index: "Point" → class[0]
            "Rectangle" → class[1]
```

### Object Instances
```
RuntimeState.instances[]
  │
  ├─ [0]: ObjectInstance (id=1)
  │   ├─ instance_id: 1
  │   ├─ class_name: "Point"
  │   └─ instance_scope: Scope
  │       ├─ X: 3.0 (member value)
  │       ├─ Y: 4.0 (member value)
  │       └─ parent_scope: GlobalScope
  │
  ├─ [1]: ObjectInstance (id=2)
  │   ├─ instance_id: 2
  │   ├─ class_name: "Point"
  │   └─ instance_scope: Scope
  │       ├─ X: 6.0
  │       ├─ Y: 8.0
  │       └─ parent_scope: GlobalScope
  │
  └─ [2]: ObjectInstance (id=3)
      ├─ instance_id: 3
      ├─ class_name: "Rectangle"
      └─ instance_scope: Scope
          ├─ Width: 5.0
          ├─ Height: 3.0
          └─ parent_scope: GlobalScope
```

## Scoping Model

```
                    ┌─────────────────────┐
                    │  GLOBAL SCOPE       │
                    │                     │
                    │ P1 (id: 1)          │
                    │ P2 (id: 2)          │
                    │ R  (id: 3)          │
                    │ DIST (value: 5.0)   │
                    └─────────────────────┘
                              ▲
                              │ lookup chain
                              │
                ┌─────────────────────────┐
                │ INSTANCE SCOPE (P1)     │
                │                         │
                │ X: 3.0 (member)         │
                │ Y: 4.0 (member)         │
                │ DX: 1.0 (local)         │
                │ DY: 2.0 (local)         │
                └────┬────────────────────┘
                     │
                     │ CALL Distance(6, 8)
                     ▼
                ┌─────────────────┐
                │ METHOD SCOPE    │
                │                 │
                │ X2: 6 (param)   │
                │ Y2: 8 (param)   │
                │ result: 5.0     │
                └─────────────────┘
```

## Method Execution Flow

```
Program:  LET DIST = P1.Distance(6, 8)
               │
               ▼
    Evaluate EXPR_MEMBER_ACCESS
               │
        ┌──────┴───────┐
        ▼              ▼
    Obj: P1        Member: Distance
    (id:1)         (method name)
        │              │
        └──────┬───────┘
               ▼
    Look up class for id:1
    (ClassDef for "Point")
               │
               ▼
    Look up "Distance" method
    (STMT_PROCEDURE_DEF)
               │
               ▼
    Push new scope
    Copy instance members (X=3, Y=4)
    Copy method parameters (X2=6, Y2=8)
               │
               ▼
    Execute method body
    (LET DX = X2 - X = 6 - 3 = 3)
    (LET DY = Y2 - Y = 8 - 4 = 4)
    (RETURN SQR(9 + 16) = 5.0)
               │
               ▼
    Pop scope
    Return value: 5.0
               │
               ▼
    DIST = 5.0
```

## Component Interaction

```
┌─────────────┐
│   LEXER     │
│             │  Reads: "CLASS Point(X,Y) PROCEDURE Distance..."
│  Produces:  │  Tokens: TOK_CLASS, TOK_IDENTIFIER("Point"), ...
└─────┬───────┘
      │ Token stream
      ▼
┌──────────────────┐
│    PARSER        │
│                  │  Builds: ASTStmt(STMT_CLASS_DEF)
│  Produces:       │           ├─ var_name: "Point"
│                  │           ├─ parameters: [X, Y]
│                  │           └─ body: Procedure nodes
└─────┬────────────┘
      │ AST
      ▼
┌──────────────────┐
│    EXECUTOR      │
│                  │  When sees STMT_CLASS_DEF:
│  Actions:        │  1. Call runtime_register_class()
│                  │  2. Store in ClassRegistry
│                  │  3. Continue execution
└─────┬────────────┘
      │
      ├─ Sees "P = NEW Point(3, 4)"
      │  └─ Evaluates EXPR_NEW
      │     └─ Calls runtime_create_instance()
      │        └─ Returns ObjectInstance (id=1)
      │        └─ Stores in instances[]
      │
      ├─ Sees "P.Distance(6, 8)"
      │  └─ Evaluates EXPR_MEMBER_ACCESS
      │     └─ Looks up instance id=1
      │     └─ Gets class "Point"
      │     └─ Finds method "Distance"
      │     └─ Creates method execution scope
      │     └─ Calls method body
      │     └─ Returns result
      │
      └─ Final result stored in DIST
```

## Memory Layout

```
RuntimeState
├─ variables[256]          ← Global variables (X, Y, DIST, P1, P2, etc.)
│  └─ var[i].name, value
├─ procedure_registry
│  └─ procedures[32] → ProcedureDef*
├─ class_registry           ← NEW in Phase 2
│  └─ classes[32] → ClassDef*
│     ├─ name: "Point"
│     ├─ parameters: ASTParameterList*
│     └─ body: ASTStmt *
├─ instances[64]            ← NEW in Phase 2
│  ├─ instances[0] → ObjectInstance
│  │  ├─ instance_id: 1
│  │  ├─ class_name: "Point"*
│  │  └─ instance_scope: Scope*
│  │     ├─ scope_id: 101
│  │     ├─ local_vars: SymbolTable* (X, Y)
│  │     └─ parent: GlobalScope
│  │
│  └─ instances[1] → ObjectInstance
│     ├─ instance_id: 2
│     ├─ class_name: "Point"*
│     └─ instance_scope: Scope*
│        ├─ scope_id: 102
│        ├─ local_vars: SymbolTable* (X, Y)
│        └─ parent: GlobalScope
│
├─ scope_stack             ← For call stack management
│  └─ Scope* current
└─ [other runtime state...]
```

## Integration With Phase 1

```
Phase 1
├─ Lexer/Parser/AST
├─ Basic types (double, string)
├─ Procedures with parameters
├─ Local scopes
└─ Runtime with variable storage

Phase 2 (Built on top)
├─ Uses all Phase 1 components
├─ Adds ClassDef + ClassRegistry
├─ Adds ObjectInstance + instance storage
├─ Methods = Procedures with object context
├─ Instances = Special variable objects
└─ Dot notation = Expression evaluation
```

## Code Flow Example

```
BasicPP Program:
    CLASS Point(X, Y)
        PROCEDURE Distance(X2, Y2)
            RETURN SQR((X2-X)^2 + (Y2-Y)^2)
        END PROCEDURE
    END CLASS
    
    LET P = NEW Point(3, 4)
    PRINT P.Distance(6, 8)

Execution:
    1. Lexer tokenizes all input
    
    2. Parser builds AST:
       - STMT_CLASS_DEF for Point
       - STMT_LET with EXPR_NEW on RHS
       - STMT_PRINT with EXPR_MEMBER_ACCESS
    
    3. Executor runs:
       a. Sees STMT_CLASS_DEF
          → runtime_register_class("Point", params, body)
          → ClassRegistry now has Point definition
       
       b. Sees STMT_LET
          → Evaluates RHS: NEW Point(3, 4)
          → EXPR_NEW → runtime_create_instance("Point")
          → Creates ObjectInstance(id=1, scope with X=3, Y=4)
          → Stores result (1.0) in variable P
       
       c. Sees STMT_PRINT
          → Evaluates EXPR_MEMBER_ACCESS
          → Object: P (= 1.0, instance id)
          → Member: Distance, Args: [6, 8]
          → Executor: Find instance 1, class Point
          → Get Distance method from Point.body
          → Push new scope with X2=6, Y2=8
          → Copy instance members X=3, Y=4 to scope
          → Execute: RETURN SQR((6-3)^2 + (8-4)^2) = SQR(25) = 5
          → Return 5.0
       
       d. PRINT outputs: 5.0
```

This layered architecture keeps Phase 2 simple while leveraging all of Phase 1's infrastructure!
