PROCEDURE Add(a, b)
    PRINT "Adding "; a; " and "; b
    LET result = a + b
    PRINT "Result: "; result
END PROCEDURE

PROCEDURE Greet(name$, greeting$)
    PRINT greeting$; " "; name$; "!"
END PROCEDURE

PRINT "Test 1: Numeric parameters"
Add(5, 3)

PRINT ""
PRINT "Test 2: String parameters"
Greet("Alice", "Hello")

PRINT ""
PRINT "Test 3: Mixed call"
Greet("Bob", "Good morning")

PRINT ""
PRINT "All tests completed!"
