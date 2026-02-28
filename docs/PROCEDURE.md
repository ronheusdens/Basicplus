# Procedures in BasicPP

## Overview

Procedures (also called subroutines or functions) are reusable blocks of code that encapsulate logic and promote code organization. BasicPP supports modern procedural programming with:

- **Parameter passing** - pass values to procedures
- **Return values** - procedures can return computed results
- **Local scope** - variables defined in procedures are local by default
- **Expression-style calls** - procedures can be used directly in expressions (OO-style)

## Procedure Definition

### Syntax

```basic
PROCEDURE ProcName(param1, param2, ...)
    REM procedure body
    REM statements here
    RETURN value
END PROCEDURE
```

### Rules

1. **Name**: Procedure names follow BASIC identifier rules (alphanumeric, start with letter)
2. **Parameters**: Optional comma-separated list of parameter names
3. **Body**: Any valid BASIC statements (nested procedures allowed)
4. **RETURN**: Optional, returns a numeric value; only valid inside procedures

## Parameter Passing

Parameters are **passed by value** and create **local variables** within the procedure scope.

### Example

```basic
PROCEDURE Add(X, Y)
    LET result = X + Y
    RETURN result
END PROCEDURE

LET A = 5
LET B = 3
LET Sum = Add(A, B)
REM A and B are still 5 and 3 (unchanged)
PRINT Sum
```

### Local Variable Isolation

Parameters are local to the procedure. Even if a global variable has the same name, it is not modified:

```basic
PROCEDURE ModifyX(X)
    LET X = X * 2
    RETURN X
END PROCEDURE

LET X = 5
LET Result = ModifyX(10)
PRINT X          REM Still 5 (global unchanged)
PRINT Result     REM 20 (returned value)
```

## Return Values

### Using RETURN Statement

The `RETURN` statement exits the procedure and returns a numeric value.

```basic
PROCEDURE Square(N)
    RETURN N * N
END PROCEDURE
```

### Expression-Style Calls (OO Style)

BasicPP supports modern object-oriented style procedure calls where the procedure call itself acts as an expression:

```basic
REM Direct assignment
LET Sq = Square(5)

REM In arithmetic expressions
LET Area = Square(Length) + Square(Width)

REM In conditions
IF Square(N) > 100 THEN
    PRINT "N > 10"
ENDIF

REM In PRINT statements
PRINT "5 squared is", Square(5)

REM Nested procedure calls
PROCEDURE Quad(N)
    RETURN Square(Square(N))
END PROCEDURE
```

### Legacy Style (Statement Form)

For backward compatibility, procedures can be called as standalone statements. The return value is stored in a global variable named `result`:

```basic
PROCEDURE CalcValue(X)
    LET result = X ^ 2
    RETURN result
END PROCEDURE

CalcValue(5)
PRINT result     REM 25
```

This form is less idiomatic; prefer expression-style calls.

## Scope Rules

### Global Variables

Variables defined at the program level (outside procedures) are global and accessible from anywhere:

```basic
LET GlobalVar = 10

PROCEDURE PrintGlobal
    PRINT GlobalVar      REM Can access global
END PROCEDURE
```

### Local Variables

Variables defined as parameters or created within a procedure are local and cease to exist when the procedure exits:

```basic
PROCEDURE CreateLocal
    LET LocalVar = 42
    PRINT LocalVar       REM 42
END PROCEDURE

CreateLocal
REM LocalVar is undefined here
PRINT LocalVar           REM Error: undefined
```

### Variable Lookup Chain

When a procedure references a variable:

1. Check if it's a parameter or local variable → use local value
2. Check if it's a global variable → use global value
3. If undefined → initialize to 0 or ""

## Practical Examples

### Example 1: Mathematical Function

```basic
PROCEDURE Factorial(N)
    IF N <= 1 THEN
        RETURN 1
    ELSE
        RETURN N * Factorial(N - 1)
    ENDIF
END PROCEDURE

PRINT Factorial(5)    REM 120
```

### Example 2: String Processing

```basic
PROCEDURE Greet(Name)
    LET Message = "Hello, " + Name
    RETURN Message
END PROCEDURE

PRINT Greet("Alice")  REM Hello, Alice
```

### Example 3: Numerical Methods (Bisection)

```basic
PROCEDURE F(X)
    RETURN X ^ 3 - 2 * X - 5
END PROCEDURE

LET A = 0
LET B = 3
FOR I = 1 TO 20
    LET Mid = (A + B) / 2
    IF F(Mid) > 0 THEN
        LET B = Mid
    ELSE
        LET A = Mid
    ENDIF
NEXT I
PRINT "Root near", (A + B) / 2
```

### Example 4: Array Processing

```basic
PROCEDURE SumArray(Arr(), N)
    LET Sum = 0
    FOR I = 1 TO N
        LET Sum = Sum + Arr(I)
    NEXT I
    RETURN Sum
END PROCEDURE

DIM Data(5)
LET Data(1) = 10
LET Data(2) = 20
LET Data(3) = 30
PRINT SumArray(Data(), 3)  REM 60
```

## Errors and Edge Cases

### Undefined Procedure

Calling an undefined procedure causes an "Illegal function call" error (code 251):

```basic
UndefinedProc()        REM Error 251
```

### RETURN Outside Procedure

The `RETURN` statement is only valid inside a procedure:

```basic
RETURN 42              REM Error: RETURN outside procedure
```

### Parameter Count Mismatch

If a procedure expects 2 parameters but receives 3, the extra is ignored. If fewer are provided, missing parameters are initialized to 0:

```basic
PROCEDURE Add(X, Y)
    RETURN X + Y
END PROCEDURE

PRINT Add(5)           REM Y defaults to 0, result = 5
PRINT Add(5, 3, 99)    REM 99 ignored, result = 8
```

## Implementation Notes

### Scope Stack

BasicPP maintains a scope stack for procedures:
- Each procedure call creates a new scope
- Parameters shadow global variables of the same name
- When the procedure exits, the scope is popped and original values are restored

### No Global Variables Within Procedures

To ensure clean procedural semantics, variables created within a procedure are local by default. To access a global variable with the same name as a local parameter, use a different variable name.

### Procedure Definitions

Procedures must be defined before they are called in the current implementation. Recursive procedures are supported.

## Summary

BasicPP procedures provide a clean, modern approach to procedural programming:

- **Define**: `PROCEDURE Name(params) ... END PROCEDURE`
- **Call**: `LET result = Name(args)` (expression-style, OO)
- **Return**: `RETURN value` (returns from procedure)
- **Scope**: Parameters and locals are local; globals accessible via lookup

This design aligns with Phase 1 of the BasicPP architecture and provides a foundation for future object-oriented enhancements in Phase 2-3.
