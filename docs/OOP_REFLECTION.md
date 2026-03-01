# BasicPP — Technical Reflection on Object-Oriented Capabilities
*Date: February 28,2026 | Scope: Phase 2 OOP Implementation*

## 1. What Has Been Built
BasicPP has a working, functional first layer of OOP. The following capabilities are implemented and tested:

### Class Definition
Classes are declared with a header that doubles as a constructor signature:

```basicpp
CLASS Point(X, Y)
    PROCEDURE Distance(X2, Y2)
        RETURN SQR((X2 - X)^2 + (Y2 - Y)^2)
    END PROCEDURE
END CLASS
```

The parameter list in the class header (`X`, `Y`) serves as the set of instance variables. There is no separate field declaration block; all per-instance state must be declared at construction time.

### Instantiation
Objects are created with `NEW`:

```basicpp
P = NEW Point(3, 4)
```

Constructor arguments are bound immediately to the named instance variables. Each object receives a unique integer ID that is stored in the variable as an opaque `double` value.

### Method Calls — Statement and Expression Forms
Both forms work:

```basicpp
P.Move(1, 2)              ' statement context
D = P.Distance(6, 8)      ' expression context (return value captured)
```

The method dispatch mechanism passes the object's instance ID as a hidden first argument. The executor detects the ID, looks up the correct class body, finds the named procedure, and executes it.

### Instance Variable Isolation
Each instance maintains independent copies of its member variables. Two `Counter` objects have separate `Count` values; modifying one does not affect the other. Multiple instances work correctly.

### Return Values from Methods
Methods can return numeric values via `RETURN expr`. The return value is captured at the call site when used in expression context.

### Context-Sensitive Parsing
Any keyword token (`PRINT`, `FOR`, `WHILE`, `RETURN`, etc.) can now be used as a class name, method name, procedure name, or variable name when the grammar context unambiguously demands an identifier. This was implemented by replacing all `TOK_IDENTIFIER` checks in declaration and name-context positions with `is_identifier_token()`.

---

## 2. How the Implementation Works Internally
### Instance Variable Storage — The Flat-Namespace Approach
Instance variables are stored in the global `RuntimeState` symbol table using name-mangled keys:

```
__INST<id>_<varname>
```

For example, instance ID 2 with member `X` is stored as `__INST2_X`.

When a method executes, the instance variables are **promoted** into the plain global namespace temporarily (saving and restoring any prior values on a procedure scope stack), giving method bodies the illusion that `X` and `Y` are normal local variables. After the method returns, modified values are written back to the mangled keys.

This works correctly for purely local access but introduces a correctness risk: if a class has a member variable with the same name as a global program variable, the method execution will shadow and potentially corrupt that global. There is no true scope isolation.

### Instance Identity
An `ObjectInstance` struct holds the class name and a unique integer ID. Objects are stored in an array on `RuntimeState`. The opaque ID (a `double`) is what gets stored in user-visible variables. Passing an object to a procedure passes the numeric ID; the callee can call methods on it just as the original owner can.

### Missing: String Instance Variables
`runtime_set_instance_string_variable()` and `runtime_get_instance_string_variable()` are **stubs** — they have empty bodies. String member variables are silently lost. Only numeric member fields work correctly.

---

## 3. What Is Missing
### 3.1 Inheritance
There is no `EXTENDS` keyword, no base-class lookup, and no method resolution order. Each class is entirely self-contained. This is the single largest missing OOP capability.

What is needed:
- Syntax: `CLASS Dog(Name) EXTENDS Animal`
- Parser: optional `EXTENDS ClassName` clause in `parse_class_def()`
- Runtime: `ClassDef` stores an optional parent name
- Method dispatch: walk the class hierarchy upward to find a method if not found in the immediate class
- Constructor chaining: call parent constructor with inherited fields

### 3.2 Polymorphism and Virtual Dispatch
Without inheritance, polymorphism is technically moot, but it is worth noting: the current dispatch is purely name-based with no concept of a vtable or method override. Two classes with a method of the same name cannot be treated interchangeably through a common interface.

### 3.3 `SELF` / `THIS` Reference
Methods have no way to refer to the object they are executing on. For example, a method cannot pass itself to another procedure, nor can it call another method on itself without accessing a global variable that holds its own ID. A `SELF` keyword would need to be injected by the executor before method body execution as an implicit variable holding the current instance ID.

### 3.4 Access Control (`PRIVATE` / `PUBLIC`)
All methods and member variables are effectively public. There is no encapsulation enforcement. A caller can read `obj.InternalData` just as freely as `obj.PublicProperty`. Adding `PRIVATE PROCEDURE` and `PRIVATE` field markers at the parser and executor level would address this.

### 3.5 String Instance Variables
As noted above, the string variant of instance variable storage is not implemented. This is a significant practical limitation — objects cannot have string fields today. Fixing this requires completing `runtime_set_instance_string_variable()` and `runtime_get_instance_string_variable()` to use the same `__INST<id>_<varname>` mangling scheme (via the string symbol table) and updating the method scope promotion/restoration logic to handle both numeric and string members.

### 3.6 Method Overloading
BASIC does not natively have a type system that supports method overloading by signature. BasicPP has not added one either. A method name uniquely identifies a procedure within a class. There is no way to define `Print()` and `Print(Width)` as two distinct methods. This is an intentional design boundary rather than an oversight, but it is worth documenting.

### 3.7 Static Methods and Class Variables
All state is per-instance. There is no mechanism for class-level (shared) variables or methods that are called without an instance. A `CLASS.StaticMethod()` syntax and a corresponding storage location in `ClassDef` (not in `ObjectInstance`) would be needed.

### 3.8 Constructors with Logic
Currently the constructor is implicit: the class parameter list is bound to the instance on `NEW`. There is no constructor body — no opportunity to compute derived fields, validate arguments, or call methods during initialization. A dedicated `PROCEDURE New()` or `PROCEDURE Init()` convention, automatically called by the `NEW` evaluator, would remedy this.

### 3.9 Object Lifetime and Memory
Once created, `ObjectInstance` structs live for the entire program run. There is no reference counting, no garbage collection, and no destructor. For long-running programs or programs that create many temporary objects, this is a memory leak. A `DELETE obj` statement could free the instance and reclaim the mangled variable names.

### 3.10 Scope Purity During Method Execution
The current "promote instance vars to global scope" strategy is a pragmatic hack. It means that a method body that reads a variable name that is also a global sees a different value than the global. The correct architecture is a proper per-instance scope that sits between the method's local scope and the program's global scope during method execution — essentially a three-level scope chain: local → instance → global.

### 3.11 Method Chaining
`obj.Move(1,2).Normalize()` style chaining — where a method returns `SELF` — is not possible today because there is no `SELF` and because the parser does not handle a member access expression as the left-hand side of another member access (though the AST `EXPR_MEMBER_ACCESS` node technically allows it in principle).

### 3.12 Type Introspection
There is no `TYPEOF(obj)` function, no `INSTANCEOF(obj, ClassName)` test, and no way to query what class an object belongs to at runtime. These would be useful for polymorphic code and defensive programming.

---

## 4. Prioritised Roadmap

Based on practical impact:

| Priority | Feature | Effort |
|---|---|---|
| 1 | String instance variables (fix stub) | Low |
| 2 | `SELF` reference in methods | Low |
| 3 | Scope chain (instance scope layer) | Medium |
| 4 | Constructor body (`PROCEDURE Init`) | Low |
| 5 | Inheritance (`EXTENDS`) | High |
| 6 | `PRIVATE` access modifier | Medium |
| 7 | Static methods / class variables | Medium |
| 8 | `DELETE` / object lifetime | Low |
| 9 | `TYPEOF` / `INSTANCEOF` | Low |
| 10 | Polymorphic dispatch | High (needs 5 first) |

### Relationship to the Original Phase Plan

The original project phases were:

| Phase | Scope | Status |
|---|---|---|
| Phase 1 | Line-number-free BASIC, procedures, structured control flow | ✅ Complete |
| Phase 2 | Classes, instantiation, method calls, instance variables | ✅ Functional (gaps remain) |
| Phase 3 | Inheritance, polymorphism, access modifiers, GC | 🔄 Replaced by this roadmap |
| Phase 4 | Compile to LLVM IR / native target | ⏸️ Not affected by this roadmap |
| Phase 5 | VS Code extension depth (debugger, LSP, variable inspector) | ⏸️ Not affected by this roadmap |

This roadmap **replaces Phase 3** entirely. It is grounded in what was actually built rather than what was designed on paper, and it re-categorises several items that were called "Phase 3" in the original plan as Phase 2 completions (items 1–4 above should have been part of Phase 2).

Phase 4 and Phase 5 remain on the original timeline and are out of scope here.

---

## 5. Summary Assessment

Phase 2 delivered a solid, working object system for purely numeric, single-inheritance-free programs. The core OOP loop — define a class, create instances, call methods, separate instance state — works correctly and cleanly. The context-sensitive parser removes the artificial restriction on using common English words as class and method names.

The principal weaknesses are architectural rather than cosmetic: the flat-namespace instance variable storage, the absence of a true scope chain, the non-functional string member variables, and the complete absence of inheritance. Of these, the string member variable gap is the most immediately impactful on real programs. The inheritance gap is the most significant from a language design perspective.

BasicPP is not yet a full OOP language in the sense that Java, Python, or C++ are OOP languages. It is better described as **BASIC with object-like structures** — capable of encapsulating state and behaviour per instance, but without the type hierarchy, encapsulation enforcement, and lifetime management that define mature OOP.
