# INT

**BASIC Set:** Level II BASIC

## Syntax
```
INT(number)
```

## Description
Returns the integer part of a number by truncating (removing) the decimal portion. This is equivalent to rounding down toward zero for both positive and negative numbers.

## Parameters
- `number` (double): The value to truncate to an integer

## Return Value
- (double): The integer part of the input number

## Example
```
10 ' Truncate positive numbers
20 PRINT INT(3.7)
30 PRINT INT(3.1)
40
50 ' Truncate negative numbers
60 PRINT INT(-3.7)
70 PRINT INT(-3.1)
80
90 ' Common use: random integer from 1 to 6 (like a die)
100 LET DICE = INT(RND(1) * 6) + 1
110 PRINT "Roll: "; DICE
120
130 ' Get integer and fractional parts
140 LET X = 7.42
150 LET INT_PART = INT(X)
160 LET FRAC_PART = X - INT_PART
170 PRINT "Integer: "; INT_PART
180 PRINT "Fraction: "; FRAC_PART
```

## Output
```
3
3
-3
-3
Roll: 5
Integer: 7
Fraction: 0.42
```

## Notes
- INT() truncates toward zero, not rounding
- For positive numbers: INT(3.9) = 3 (not 4)
- For negative numbers: INT(-3.9) = -3 (not -4)
- Useful for converting floating-point results to integers
- Combined with RND(), creates random integers in any range
- To round to nearest integer, use INT(X + 0.5) for positive numbers

## Algorithm Examples
- Random integer 1-10: INT(RND(1) * 10) + 1
- Random integer 1-6: INT(RND(1) * 6) + 1
- Extract integer part: INT(number)
- Extract fractional part: number - INT(number)

## Related Functions
- `RND()`: Random number generator (returns 0 to 1)
- `ABS()`: Absolute value
- `SGN()`: Sign of a number (-1, 0, or 1)
