# SGN

**BASIC Set:** Level II BASIC

## Syntax
```
SGN(number)
```

## Description
Returns the sign of a number as an integer value. This function is useful for determining whether a number is positive, negative, or zero without needing conditional statements.

## Parameters
- `number` (double): The value to determine the sign of

## Return Value
- (double): One of three values:
  - `1` if the number is positive (> 0)
  - `0` if the number is zero
  - `-1` if the number is negative (< 0)

## Example
```
10 ' Test sign of various numbers
20 PRINT "SGN(-5) = "; SGN(-5)
30 PRINT "SGN(0) = "; SGN(0)
40 PRINT "SGN(3.14) = "; SGN(3.14)
50
60 ' Determine if a number is positive, negative, or zero
70 INPUT "Enter a number: "; N
80 LET S = SGN(N)
90 IF S > 0 THEN PRINT "Number is positive"
100 IF S = 0 THEN PRINT "Number is zero"
110 IF S < 0 THEN PRINT "Number is negative"
120
130 ' Use SGN to build a comparison value
140 LET A = 10
150 LET B = 20
160 LET COMPARISON = SGN(B - A)
170 PRINT "Comparison: "; COMPARISON; " (negative if B<A, zero if B=A, positive if B>A)"
```

## Output
```
SGN(-5) = -1
SGN(0) = 0
SGN(3.14) = 1
Enter a number: 5
Number is positive
Comparison: 1 (negative if B<A, zero if B=A, positive if B>A)
```

## Notes
- SGN() returns a double value, not an integer (e.g., 1.0, not 1)
- Useful for avoiding complex IF...THEN statements
- Works with any real number, including very large and very small values
- SGN(0.0) always returns 0
- Can be used to normalize direction or determine ordering

## Algorithm Examples
- Determine ordering: IF SGN(A - B) > 0 THEN ... (equivalent to IF A > B)
- Get absolute value sign: ABS(X) * SGN(X) = X
- Compare three values: COMPARISON = SGN(B - A) returns -1, 0, or 1

## Related Functions
- `ABS()`: Absolute value (magnitude without sign)
- `INT()`: Integer part of number
- `RND()`: Random number generator
