# ABS

**BASIC Set:** Level II BASIC

## Syntax
```
ABS(number)
```

## Description
Returns the absolute value (magnitude) of a number, removing the sign. The result is always non-negative.

## Parameters
- `number` (double): The value to get the absolute value of

## Return Value
- (double): The absolute value of the input number

## Example
```
10 LET X = -42
20 PRINT ABS(X)
30
40 LET Y = 3.14
50 PRINT ABS(Y)
60
70 LET Z = 0
80 PRINT ABS(Z)
90
100 ' ABS is useful for distance calculations
110 LET A = -10
120 LET B = 5
130 PRINT "Distance: "; ABS(A - B)
```

## Output
```
42
3.14
0
Distance: 15
```

## Notes
- ABS(-5) equals ABS(5): both return 5
- ABS(0) returns 0
- Useful for calculating distances, differences, and error magnitudes
- Often used with RND for seeding: RND(-ABS(X))

## Related Functions
- `SGN()`: Returns the sign of a number (-1, 0, or 1)
- `INT()`: Truncates to integer
- `RND()`: Random number generator (uses negative values for seeding)
