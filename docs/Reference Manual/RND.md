# RND

**BASIC Set:** Level II BASIC (Microsoft BASIC)

## Syntax
```
RND(argument)
```

## Description
Returns a pseudo-random number between 0 (inclusive) and 1 (exclusive). The function uses a Linear Congruential Generator (LCG) algorithm compatible with TRS-80 Level II BASIC.

- **RND(0)**: Returns the last generated random number
- **RND(positive)**: Generates and returns the next random number in the sequence
- **RND(negative)**: Reseeds the generator with the absolute value and returns the first random number from the new sequence

## Parameters
- `argument` (double): Control parameter for the RND function
  - `0`: Return last value
  - `> 0`: Generate next random number
  - `< 0`: Reseed and generate

## Algorithm
The implementation uses a 16-bit Linear Congruential Generator:

X_{n+1} = (75 * X_n + 74) mod 65536

The returned value is: RND = X_n / 65536

This gives values in the range [0, 1) (0 inclusive, 1 exclusive).

## Example
```
10 ' Generate random numbers
20 LET R1 = RND(1)
30 PRINT "Random 1: "; R1
40 LET R2 = RND(1)
50 PRINT "Random 2: "; R2
60
70 ' Get last value
80 LET R3 = RND(0)
90 PRINT "Last value: "; R3
100
110 ' Reseed with -42 and generate new sequence
120 LET R4 = RND(-42)
130 PRINT "Reseeded: "; R4
140
150 ' Reseed again with same value
160 LET R5 = RND(-42)
170 PRINT "Same seed gives: "; R5
180 IF R5 = R4 THEN PRINT "Sequences are repeatable"
```

## Output
```
Random 1: 0.76251220703125
Random 2: 0.189544677734375
Last value: 0.189544677734375
Reseeded: 0.0491943359375
Same seed gives: 0.0491943359375
Sequences are repeatable
```

## Notes
- The seeding uses only 16 bits of the argument value due to the LCG state width
- Different seeds will produce different sequences, enabling controllable randomization
- The same seed always produces the same sequence (deterministic)
- RND(0) is useful for getting the current value without advancing the sequence
- Results are always in the range [0, 1); multiply by a constant to get ranges like [1, 7] for dice rolls
- This implementation is compatible with TRS-80 Level II BASIC programs that use RND

## Related Functions
- `INT()`: Convert to integer for dice rolls or random selection
- `ABS()`: Get absolute value for seed conversion
