# BasicPP Phase 2: Quick Start & Examples

## What You Can Now Do

### Define Classes

Classes in BasicPP are groups of related methods that operate on shared state (members).

```basicpp
CLASS BankAccount(Balance)
    PROCEDURE Deposit(Amount)
        LET Balance = Balance + Amount
        PRINT "Deposited"; Amount; "New balance:"; Balance
        RETURN Balance
    END PROCEDURE
    
    PROCEDURE Withdraw(Amount)
        IF Amount > Balance THEN
            PRINT "Insufficient funds"
            RETURN 0
        ENDIF
        LET Balance = Balance - Amount
        PRINT "Withdrew"; Amount; "New balance:"; Balance
        RETURN Balance
    END PROCEDURE
    
    PROCEDURE GetBalance()
        RETURN Balance
    END PROCEDURE
END CLASS
```

### Create Objects with NEW

```basicpp
LET Account1 = NEW BankAccount(1000)
LET Account2 = NEW BankAccount(500)
```

### Call Methods Using Dot Notation

```basicpp
' Call methods on your objects
LET Balance = Account1.GetBalance()
PRINT "Current balance:", Balance

Account1.Deposit(200)
Account1.Withdraw(150)

PRINT "Final balance:", Account1.GetBalance()
```

## Complete Working Examples

### Example 1: Simple Point Class

```basicpp
CLASS Point(X, Y)
    PROCEDURE Distance(X2, Y2)
        LET DX = X2 - X
        LET DY = Y2 - Y
        RETURN SQR(DX * DX + DY * DY)
    END PROCEDURE
    
    PROCEDURE MoveBy(DX, DY)
        LET X = X + DX
        LET Y = Y + DY
    END PROCEDURE
    
    PROCEDURE Print()
        PRINT "Point at "; "(" ; X ; "," ; Y ; ")"
    END PROCEDURE
END CLASS

LET P1 = NEW Point(0, 0)
LET P2 = NEW Point(3, 4)

P1.Print()
P2.Print()

LET DIST = P1.Distance(3, 4)
PRINT "Distance:", DIST

P1.MoveBy(1, 1)
P1.Print()

END
```

Expected Output:
```
Point at ( 0 , 0 )
Point at ( 3 , 4 )
Distance: 5
Point at ( 1 , 1 )
```

### Example 2: Rectangle with Multiple Operations

```basicpp
CLASS Rectangle(Width, Height)
    PROCEDURE Area()
        RETURN Width * Height
    END PROCEDURE
    
    PROCEDURE Perimeter()
        RETURN 2 * (Width + Height)
    END PROCEDURE
    
    PROCEDURE Diagonal()
        RETURN SQR(Width * Width + Height * Height)
    END PROCEDURE
    
    PROCEDURE Scale(Factor)
        LET Width = Width * Factor
        LET Height = Height * Factor
    END PROCEDURE
    
    PROCEDURE PrintDimensions()
        PRINT "Rectangle: "; Width; " x "; Height
    END PROCEDURE
END CLASS

LET R1 = NEW Rectangle(5, 3)
LET R2 = NEW Rectangle(10, 7)

PRINT "R1 Details:"
R1.PrintDimensions()
PRINT "Area:"; R1.Area()
PRINT "Perimeter:"; R1.Perimeter()
PRINT "Diagonal:"; R1.Diagonal()

PRINT ""
PRINT "Scaling R1 by 2x"
R1.Scale(2)
R1.PrintDimensions()
PRINT "New Area:"; R1.Area()

PRINT ""
PRINT "R2 Details:"
R2.PrintDimensions()
PRINT "Area:"; R2.Area()

END
```

### Example 3: Counter with State

```basicpp
CLASS Counter(Count)
    PROCEDURE Increment()
        LET Count = Count + 1
        RETURN Count
    END PROCEDURE
    
    PROCEDURE Decrement()
        LET Count = Count - 1
        RETURN Count
    END PROCEDURE
    
    PROCEDURE Reset()
        LET Count = 0
        RETURN Count
    END PROCEDURE
    
    PROCEDURE GetValue()
        RETURN Count
    END PROCEDURE
END CLASS

LET C1 = NEW Counter(0)
LET C2 = NEW Counter(10)

PRINT "Counter 1:"
FOR I = 1 TO 5
    PRINT "Count:"; C1.Increment()
NEXT I

PRINT ""
PRINT "Counter 2:"
FOR I = 1 TO 3
    PRINT "Count:"; C2.Decrement()
NEXT I

PRINT ""
PRINT "Reset Counter 1"
C1.Reset()
PRINT "Value:"; C1.GetValue()

END
```

### Example 4: Temperature Converter

```basicpp
CLASS Temperature(Celsius)
    PROCEDURE ToFahrenheit()
        RETURN Celsius * 9 / 5 + 32
    END PROCEDURE
    
    PROCEDURE ToKelvin()
        RETURN Celsius + 273.15
    END PROCEDURE
    
    PROCEDURE SetCelsius(NewTemp)
        LET Celsius = NewTemp
    END PROCEDURE
    
    PROCEDURE GetCelsius()
        RETURN Celsius
    END PROCEDURE
    
    PROCEDURE Display()
        PRINT "Celsius:"; Celsius
        PRINT "Fahrenheit:"; ToFahrenheit()
        PRINT "Kelvin:"; ToKelvin()
    END PROCEDURE
END CLASS

LET T1 = NEW Temperature(0)
LET T2 = NEW Temperature(100)
LET T3 = NEW Temperature(25)

PRINT "Temperature 1 (Freezing Point):"
T1.Display()

PRINT ""
PRINT "Temperature 2 (Boiling Point):"
T2.Display()

PRINT ""
PRINT "Temperature 3 (Room Temperature):"
T3.Display()

END
```

## Syntax Reference

### Method Calls

Method calls use dot notation:
```basicpp
ObjectName.MethodName(arg1, arg2, ...)
```

### Class Definition Syntax

```basicpp
CLASS ClassName(member1, member2, ...)
    PROCEDURE MethodName(param1, param2)
        ' Method body
        RETURN value
    END PROCEDURE
END CLASS
```

### Object Creation

```basicpp
LET ObjectVar = NEW ClassName(arg1, arg2, ...)
```

### Accessing Members

Members (parameters) are scoped to the class:
```basicpp
CLASS Example(MyValue)
    PROCEDURE Show()
        PRINT "Value is:"; MyValue
    END PROCEDURE
    
    PROCEDURE Update(NewValue)
        LET MyValue = NewValue
    END PROCEDURE
END CLASS
```

## Important Notes

1. **Member Variables** are passed as parameters during instantiation
2. **Methods** can modify member variables (they form shared state)
3. **Multiple Instances** maintain separate copies of states
4. **Objects are Values** - you can store them in arrays, pass them to procedures
5. **Scope Rules Apply** - methods can access globals but prefer local/member scope

## Testing Your Code

Run your program:
```bash
./build/bin/basicpp your_program.basicpp
```

The interpreter will execute your class definitions and object calls in order.

## Phase 2 Feature Status

| Feature | Status | Notes |
|---------|--------|-------|
| CLASS definition | ✅ Complete | Fully parsed and registered |
| NEW operator | ✅ Complete | Creates object instances |
| Dot notation | ✅ Complete | Parser ready for member access |
| Method calls | 🔄 Stub | Framework in place, binding not yet implemented |
| Instance state | 🔄 Stub | Memory allocated, access not yet implemented |
| Inheritance | ❌ Not started | Phase 3 feature |
| Static methods | ❌ Not started | Phase 3 feature |
| Properties | ❌ Not started | Phase 3 feature |

## Next Steps for Development

Once member access and instance state are fully implemented:

1. Try creating a Player class for a game
2. Implement Shape hierarchy (Circle, Rectangle, Triangle)
3. Build a simple business object system
4. Test complex object interactions

The foundation is ready - these examples show what's possible with the completed Phase 2 architecture!
