# Basic++ Grammar Specification (Phase 1)

## Overview

Basic++ is an object-oriented extension of BASIC that eliminates line numbers and GOTO/GOSUB control flow in favor of structured procedures, local scoping, and clean block-based syntax.

**Phase 1 Goal:** Line-number-free interpreter with procedure support and no jumps.

---

## Lexical Syntax

### Keywords

**Control Flow:**
- PROCEDURE, END, RETURN
- IF, THEN, ELSE, ENDIF
- FOR, NEXT
- WHILE, WEND
- DO, LOOP, EXIT, CONTINUE

**Data Types:**
- INTEGER, LONG, SINGLE, DOUBLE, STRING, BOOLEAN, VARIANT
- AS (type annotation)
- DIM (array declaration)

**Operations:**
- LET (assignment)
- PRINT, INPUT
- READ, DATA, RESTORE

**Comments:**
- REM ... (line comment)
- ' ... (line comment)

### Identifiers

- Start with letter or underscore: `[a-zA-Z_]`
- Continue with alphanumerics or underscore: `[a-zA-Z0-9_]*`
- Optional type suffix: `$` (string), `%` (long), `!` (single), `#` (double)
- Example: `MyVariable`, `Name$`, `Count%`, `_privateVar`

### Literals

- **Integer:** `123`, `-45`, `0`
- **Float:** `3.14`, `1.5e-10`, `-2.3E+5`
- **String:** `"Hello"`, `"Line 1\nLine 2"`
- **Boolean:** `TRUE`, `FALSE` (later)

### Operators

**Arithmetic:** `+`, `-`, `*`, `/`, `\` (integer div), `^` (power), `MOD`

**Comparison:** `=`, `<>`, `<`, `<=`, `>`, `>=`

**Logical:** `AND`, `OR`, `NOT`, `XOR`, `EQV`, `IMP`

**Assignment:** `=`, `LET ... =`

---

## Grammar (EBNF)

### Program Structure

```
Program := (ProcedureDefinition | Statement)*

ProcedureDefinition := 'PROCEDURE' Identifier '(' ParameterList? ')'
                       StatementBlock
                       'END' 'PROCEDURE'

ParameterList := Identifier (',' Identifier)*

StatementBlock := Statement+
```

### Statements

```
Statement := VariableDeclaration
           | Assignment
           | PrintStatement
           | InputStatement
           | IfStatement
           | ForStatement
           | WhileStatement
           | DoStatement
           | ProcedureCall
           | ReturnStatement
           | DataStatement
           | ReadStatement
           | RestoreStatement

VariableDeclaration := 'DIM' Identifier 'AS' Type
                     | 'DIM' Identifier '(' DimensionList ')' [ 'AS' Type ]

Type := 'INTEGER' | 'LONG' | 'SINGLE' | 'DOUBLE' | 'STRING' | 'BOOLEAN' | 'VARIANT'

DimensionList := Expression (',' Expression)*

Assignment := Variable '=' Expression
            | 'LET' Variable '=' Expression

PrintStatement := 'PRINT' ExpressionList?
                | 'PRINT' String [ ';' ExpressionList ]

ExpressionList := Expression (';' Expression | ',' Expression)*

InputStatement := 'INPUT' VariableList
               | 'INPUT' String ';' VariableList

VariableList := Variable (',' Variable)*

IfStatement := 'IF' Expression 'THEN'
               StatementBlock
               ('ELSE'
                StatementBlock)?
               'ENDIF'

ForStatement := 'FOR' Variable '=' Expression 'TO' Expression ['STEP' Expression]
                StatementBlock
                'NEXT' Variable

WhileStatement := 'WHILE' Expression
                  StatementBlock
                  'WEND'

DoStatement := 'DO'
               StatementBlock
               'LOOP' [UntilCondition]

UntilCondition := 'UNTIL' Expression | 'WHILE' Expression

ProcedureCall := Identifier '(' ArgumentList? ')'

ArgumentList := Expression (',' Expression)*

ReturnStatement := 'RETURN'
                | 'RETURN' Expression

DataStatement := 'DATA' LiteralList

LiteralList := Literal (',' Literal)*

ReadStatement := 'READ' VariableList

RestoreStatement := 'RESTORE'
```

### Expressions

```
Expression := LogicalExpression

LogicalExpression := RelationalExpression
                   ( ('OR' | 'XOR' | 'EQV' | 'IMP') RelationalExpression )*

RelationalExpression := AdditiveExpression
                      ( ('=' | '<>' | '<' | '<=' | '>' | '>=') AdditiveExpression )*

AdditiveExpression := MultiplicativeExpression
                    ( ('+' | '-') MultiplicativeExpression )*

MultiplicativeExpression := ExponentialExpression
                          ( ('*' | '/' | '\' | 'MOD') ExponentialExpression )*

ExponentialExpression := UnaryExpression
                       ( '^' UnaryExpression )*

UnaryExpression := ('NOT' | '-' | '+') UnaryExpression
                 | PostfixExpression

PostfixExpression := PrimaryExpression
                   ( '[' Expression (',' Expression)* ']'  // array subscript
                   | '(' ArgumentList? ')'                  // function call
                   )*

PrimaryExpression := Identifier
                   | Literal
                   | '(' Expression ')'
                   | FunctionCall
                   | ArrayLiteral

FunctionCall := Identifier '(' ArgumentList? ')'

ArrayLiteral := '[' ExpressionList? ']'  // future feature
```

---

## Type System

### Implicit Types (via suffix)

| Suffix | Type | Example |
|--------|------|---------|
| (none) | DOUBLE | `PI = 3.14159` |
| `$` | STRING | `Name$ = "John"` |
| `%` | LONG | `Count% = 100` |
| `!` | SINGLE | `Value! = 1.5` |
| `#` | DOUBLE | `X# = 2.718` |

### Explicit Types (via DIM/AS)

```basic
DIM Name AS STRING
DIM Array(10) AS INTEGER
PROCEDURE Calculate(X AS DOUBLE) AS DOUBLE
```

### Type Coercion

- STRING + DOUBLE → STRING (concatenation)
- DOUBLE + STRING → STRING (concatenation)
- INTEGER operations → LONG
- LONG + SINGLE → DOUBLE
- Mixed numeric → DOUBLE

---

## Scoping Rules

### Globals

- Declared at program level
- Accessible from any procedure
- Lifetime: program execution

```basic
DIM GlobalX AS DOUBLE
PROCEDURE Update()
  LET GlobalX = 10  ' visible here
END PROCEDURE
```

### Locals

- Declared inside procedures
- Scoped to procedure body
- Shadow globals if same name
- Lifetime: procedure execution

```basic
PROCEDURE Test()
  DIM LocalX AS DOUBLE       ' local to Test()
  LET LocalX = 5             ' shadows GlobalX if it exists
END PROCEDURE
```

### Parameters

- Local to procedure
- Passed by value (scalars) or reference (arrays)
- Lifetime: procedure execution

```basic
PROCEDURE Calculate(X AS DOUBLE, Result AS DOUBLE)
  ' X and Result are local parameters
END PROCEDURE
```

---

## Built-in Functions (Phase 1)

### Math Functions
- ABS(x), SQR(x), SIN(x), COS(x), TAN(x)
- ASIN(x), ACOS(x), ATAN(x)
- LOG(x), LN(x), EXP(x)
- INT(x), FIX(x), ROUND(x, places)
- RND(), RANDOMIZE()

### String Functions
- LEN(s), LEFT$(s, n), RIGHT$(s, n), MID$(s, start, len)
- UPPER$(s), LOWER$(s)
- STR$(x), VAL(s)
- TRIM$(s), LTRIM$(s), RTRIM$(s)
- INSTR(s, sub), REPLACE$(s, old, new)

### I/O Functions
- PRINT, INPUT
- READ, DATA, RESTORE
- (File I/O deferred to Phase 2+)

### Type Functions
- TYPEOF(x) → STRING
- ISARRAY(x) → BOOLEAN
- ISNUMBER(x) → BOOLEAN

---

## Example Program (Phase 1)

```basic
' Binary search - no line numbers, no GOTO/GOSUB
PROCEDURE Evaluate(X AS DOUBLE) AS DOUBLE
  RETURN X^5 + 2*X^3 - 1
END PROCEDURE

PROCEDURE BinarySearch(A AS DOUBLE, B AS DOUBLE, N AS INTEGER) AS DOUBLE
  FOR I = 1 TO N
    DIM MidPoint AS DOUBLE
    LET MidPoint = (A + B) / 2
    
    IF Evaluate(MidPoint) > 0 THEN
      LET B = MidPoint
    ELSE
      LET A = MidPoint
    ENDIF
  NEXT I
  
  RETURN (A + B) / 2
END PROCEDURE

' Main program
DIM Result AS DOUBLE
LET Result = BinarySearch(0, 1, 20)
PRINT "Root: "; Result
PRINT "Verify: "; Evaluate(Result)
```

---

## Parser Strategy

1. **Tokenization:** Lexer produces token stream (no line-number requirement)
2. **Top-level parsing:** Parse procedures and statements at program level
3. **Block parsing:** Recursive descent for nested structures
4. **Scope tracking:** Maintain scope stack during parsing/execution
5. **AST building:** Create procedure/statement nodes with proper containment

---

## Implementation Priorities

### Must Have (Phase 1)
- ✅ Procedure definitions and calls
- ✅ Local variable scoping
- ✅ IF/THEN/ELSE/ENDIF blocks
- ✅ FOR/NEXT loops
- ✅ WHILE/WEND loops
- ✅ Expression evaluation
- ✅ PRINT statement
- ✅ INPUT statement
- ✅ Basic math functions
- ✅ String functions (basic)
- ✅ Type suffixes

### Should Have (Phase 1)
- DO/LOOP
- RETURN from procedures
- DIM with AS syntax
- EXIT statement
- Array support (read-only)

### Nice to Have (Phase 2+)
- CALL statement
- File I/O (OPEN, CLOSE, PUT, GET)
- Record-based data
- Classes and methods (Phase 2)
- Inheritance (Phase 3)

---

## Notes

- All keywords are case-insensitive
- Whitespace and newlines are flexible (can use `:` for inline statements)
- Indentation is NOT significant (but recommended for readability)
- No implicit line continuation needed (explicit statements via BEGIN/END or PROCEDURE/END PROCEDURE)
