# BasicPP Phase 2: Object-Oriented Programming

**Status:** Architecture Complete, Foundation Implemented
**Date:** February 28, 2026

## Overview

Phase 2 extends BasicPP with object-oriented features centered around **classes and methods**. The design follows a clean OOP model where:
- **Classes** group related methods and member data
- **Methods** are procedures that operate on instance state
- **Instances** (objects) are created with `NEW ClassName(...)`
- **Member access** uses dot notation: `obj.method()`

## Architecture

### 1. Lexer (src/lexer.c/h)

**New Tokens:**
- `TOK_CLASS` - CLASS keyword
- `TOK_NEW` - NEW keyword for instantiation
- `TOK_DOT` - Dot (.) for member access

**Changes:**
- Added keyword recognition for CLASS and NEW
- Added dot operator tokenization (.lex line ~390)

### 2. AST (src/ast.h/c)

**New Statement Type:**
```c
STMT_CLASS_DEF  // Class definition
```

**New Expression Types:**
```c
EXPR_MEMBER_ACCESS  // obj.field or obj.method()
EXPR_NEW            // NEW ClassName(...)
```

**Extended ASTStmt Structure:**
```c
struct ASTStmt {
    // ... existing fields ...
    
    /* Class-related fields (for STMT_CLASS_DEF) */
    ASTParameterList *members;     // Member variable declarations
    char **method_names;            // Names of method procedures
    int num_methods;
    int capacity_methods;
}
```

**Extended ASTExpr Structure:**
```c
struct ASTExpr {
    // ... existing fields ...
    
    /* Member access fields (for EXPR_MEMBER_ACCESS) */
    ASTExpr *member_obj;    // Object expression (left of dot)
    char *member_name;      // Member name (right of dot)
}
```

### 3. Runtime (src/runtime.h/c)

**New Types:**
```c
typedef struct ClassDef {
    char *name;               // Class name
    void *parameters;         // ASTParameterList ptr - member variables
    void *body;              // ASTStmt ptr - method definitions
    void *method_procedures; // Parsed procedure definitions
} ClassDef;

typedef struct {
    ClassDef **classes;      // Array of class definitions
    int count;               // Number of classes
    int capacity;            // Allocated capacity
} ClassRegistry;

typedef struct {
    char *class_name;        // Class this instance belongs to
    void *instance_scope;    // Scope ptr - member variables
    int instance_id;         // Unique ID for this instance
} ObjectInstance;
```

**RuntimeState Extensions:**
```c
struct RuntimeState {
    // ... existing fields ...
    
    ClassRegistry *class_registry;      // All registered classes
    ObjectInstance **instances;         // Active object instances
    int num_instances;
    int capacity_instances;
    int next_instance_id;              // Counter for unique IDs
}
```

**New Runtime Functions:**

Class registry management:
```c
ClassRegistry *class_registry_create(void);
void class_registry_free(ClassRegistry *reg);
void class_registry_add(ClassRegistry *reg, const char *name, void *params, void *body);
ClassDef *class_registry_lookup(ClassRegistry *reg, const char *name);
```

RuntimeState class access:
```c
void runtime_register_class(RuntimeState *state, const char *name, void *params, void *body);
ClassDef *runtime_lookup_class(RuntimeState *state, const char *name);
ClassRegistry *runtime_get_class_registry(RuntimeState *state);
```

Object instance management:
```c
ObjectInstance *runtime_create_instance(RuntimeState *state, const char *class_name);
void runtime_free_instance(ObjectInstance *instance);
ObjectInstance *runtime_get_instance(RuntimeState *state, int instance_id);
void runtime_set_instance_variable(ObjectInstance *inst, const char *var_name, double value);
double runtime_get_instance_variable(ObjectInstance *inst, const char *var_name);
// ... string variants
```

### 4. Parser (src/parser.c)

**New Parser Function:**
```c
static ASTStmt *parse_class_def(Parser *parser)
```

**Parsing Strategy:**

```
CLASS ClassName(member1, member2, ...)
    PROCEDURE Method1(param1, param2)
        ' method body
    END PROCEDURE
    
    PROCEDURE Method2(param1)
        ' method body
    END PROCEDURE
END CLASS
```

**Key Features:**
- Parses CLASS keyword and class name
- Optional member parameters in parentheses (like constructor args)
- Expects PROCEDURE definitions as methods inside class body
- Handles nested scoping properly
- Validates END CLASS terminator

### 5. Executor (src/executor.c)

**New Statement Handler:**
```c
case STMT_CLASS_DEF:
    // Store class in registry during program execution
    runtime_register_class(ctx->runtime, stmt->var_name, 
                          (void *)stmt->parameters, 
                          (void *)stmt->body);
    break;
```

**Processing:**
1. When a CLASS statement is encountered during execution, it's stored in the class registry
2. Classes can be instantiated later with `NEW ClassName(...)`
3. Instance creation pushes a new scope for member variables

### 6. Evaluator (src/eval.c)

**New Expression Handlers:**

```c
case EXPR_NEW:
    // NEW ClassName(...) - creates object instance
    if (expr->var_name && state) {
        ObjectInstance *inst = runtime_create_instance(ctx->runtime, expr->var_name);
        if (inst) {
            return (double)inst->instance_id;  // Return ID as numeric value
        }
    }
    break;

case EXPR_MEMBER_ACCESS:
    // obj.field or obj.method() - member access via dot notation
    // Evaluates member_obj, then looks up member_name
    // Returns member value (or calls method if callable)
    break;
```

## Syntax Examples

### Basic Class Definition
```basicpp
CLASS Point(X, Y)
    PROCEDURE Distance(X2, Y2)
        LET DX = X2 - X
        LET DY = Y2 - Y
        RETURN SQR(DX^2 + DY^2)
    END PROCEDURE
END CLASS
```

### Object Creation
```basicpp
LET P1 = NEW Point(3, 4)
LET P2 = NEW Point(6, 8)
```

### Method Calls
```basicpp
LET DIST = P1.Distance(P2_X, P2_Y)
PRINT "Distance:", DIST
```

### Multiple Methods
```basicpp
CLASS Rectangle(W, H)
    PROCEDURE Area()
        RETURN W * H
    END PROCEDURE
    
    PROCEDURE Perimeter()
        RETURN 2 * (W + H)
    END PROCEDURE
END CLASS

LET R = NEW Rectangle(5, 3)
PRINT R.Area()           ' Outputs 15
PRINT R.Perimeter()      ' Outputs 16
```

## Implementation Status

### ✅ Complete
- Lexer: CLASS, NEW, DOT tokens recognized
- AST: STMT_CLASS_DEF, EXPR_NEW, EXPR_MEMBER_ACCESS types defined
- Parser: parse_class_def() implemented, CLASS parsing in parse_program()
- Runtime: ClassRegistry and ObjectInstance types created
- Runtime: Instance lifecycle management (create, free, lookup)
- Executor: CLASS_DEF statement handler registered
- Evaluator: EXPR_NEW and EXPR_MEMBER_ACCESS stubs in place

### 🔄 In Progress / Stub Implementation
- Member variable access (instance-specific state)
- Method call binding (obj.method() routing)
- Constructor argument handling
- Inheritance (not in Phase 2 scope)

### ⏸️ Future Work (Phase 3)
- Inheritance and polymorphism
- Static methods
- Property syntax (obj.property as sugar for obj.GetProperty())
- Access modifiers (Public/Private)
- Object serialization
- Garbage collection (automatic cleanup)

## Memory Management

**Class Definitions:**
- Stored in ClassRegistry, freed on program exit
- Parameters and body managed by AST

**Object Instances:**
- Each instance maintains live reference in RuntimeState.instances
- Instance scope owns local variable storage
- Freed explicitly or on program exit

**Variable Scope:**
- Global scope: RuntimeState.variables
- Procedure scope: ScopeStack per call
- Instance scope: ObjectInstance.instance_scope

## Integration Points

The Phase 2 implementation integrates cleanly with Phase 1:

1. **Procedures remain unchanged** - Methods are just procedures with implicit "this" context
2. **Scoping rules apply** - Instance methods can access:
   - Instance variables (parameters)
   - Local method variables
   - Global variables (lookup chain)
3. **Type system unchanged** - Objects stored as numeric IDs (like arrays)
4. **Error handling unchanged** - ON ERROR applies to object operations

## Testing

**Test Files Created:**
- `tests/test_phase2_classes.basicpp` - Basic class/method syntax
- `tests/test_member_access.basicpp` - Dot notation member access

**Running Tests:**
```bash
cd /Users/ronheusdens/Stack/Dev/MacSilicon/c/basic++
./build/bin/basicpp tests/test_phase2_classes.basicpp
./build/bin/basicpp tests/test_member_access.basicpp
```

## Compilation

All code compiles without errors. Minor warnings about unused parameters in stub functions (expected for Phase 2 foundation).

```bash
make clean && make
# ✓ Built: build/bin/basicpp
```

## Design Rationale

### Why Procedures as Methods?
Methods are syntactic sugar for scoped procedures. By reusing the procedure infrastructure, we maintain:
- Single execution model
- Consistent parameter passing
- Same scoping rules
- Backward compatibility

### Why Instance IDs as Values?
Objects are represented as double values (instance IDs) to:
- Maintain BasicPP's simple type system (no pointer types)
- Allow object references in arrays and variables
- Keep runtime simple (no type tagging needed)
- Enable easy serialization

### Why Dot Notation?
Object.method syntax follows modern language conventions and maps well to:
- Member access as expression
- Method calls as function calls with implicit object context
- Future property syntax (`obj.field` as sugar)

## Next Steps

The foundation for full OOP is now in place. Phase 2 direction options:

1. **Complete Member Access** (Recommended)
   - Implement EXPR_MEMBER_ACCESS evaluation
   - Route method calls through member access
   - Add instance variable binding

2. **Extend to Full Inheritance** (Skip to Phase 3)
   - Implement base class declarations
   - Support virtual methods
   - Add super() calls

3. **Add Static Methods/Properties** (Alternative)
   - Class-level variables and methods
   - Singleton patterns
   - Utility classes

## Files Modified

```
src/lexer.h                 +3 tokens (CLASS, NEW, DOT)
src/lexer.c                 +keyword table entries + dot tokenization
src/ast.h                   +STMT_CLASS_DEF, EXPR_*
src/parser.c                +parse_class_def() function
src/executor.c              +STMT_CLASS_DEF handler
src/eval.c                  +EXPR_NEW, EXPR_MEMBER_ACCESS handlers
src/runtime.h               +ClassDef, ClassRegistry, ObjectInstance types
src/runtime.c               +class registry + object instance management
tests/test_phase2_classes.basicpp           +3 test programs
tests/test_member_access.basicpp
```

**Total Lines Added:** ~600 (foundation + framework)
**Architecture Complete:** ✓
