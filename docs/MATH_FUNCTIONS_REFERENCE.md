# Mathematical Functions Reference

**TRS-80 Level II BASIC Interpreter**  
**Version**: 1.4.0  
**Date**: 2026-01-19

---

## Complete Function List

All standard TRS-80 Model 3/4 Disk BASIC mathematical functions are now supported.

### Basic Functions

| Function | Description | Example | Result |
|----------|-------------|---------|--------|
| `ABS(x)` | Absolute value | `ABS(-5)` | `5` |
| `SGN(x)` | Sign of number | `SGN(-5)` | `-1` |
| `SQR(x)` | Square root | `SQR(25)` | `5` |
| `INT(x)` | Floor (round down) | `INT(3.7)` | `3` |
| `FIX(x)` | Truncate towards zero | `FIX(-3.7)` | `-3` |
| `CINT(x)` | Round to nearest integer | `CINT(3.5)` | `4` |

### Trigonometric Functions (Radians)

| Function | Description | Example | Result |
|----------|-------------|---------|--------|
| `SIN(x)` | Sine | `SIN(0)` | `0` |
| `COS(x)` | Cosine | `COS(0)` | `1` |
| `TAN(x)` | Tangent | `TAN(0)` | `0` |
| `ATN(x)` | Arctangent | `ATN(1)` | `0.7854` (π/4) |

### Exponential and Logarithmic

| Function | Description | Example | Result |
|----------|-------------|---------|--------|
| `EXP(x)` | e^x | `EXP(1)` | `2.718` |
| `LOG(x)` | Log base 10 | `LOG(100)` | `2` |

### Random Numbers

| Function | Description | Example | Result |
|----------|-------------|---------|--------|
| `RND(x)` | Random 0-1 | `RND(0)` | `0.xxxxx` |
| `RANDOM` | Random 0-1 | `RANDOM(0)` | `0.xxxxx` |

### Type Conversion

| Function | Description | Example | Result |
|----------|-------------|---------|--------|
| `CDBL(x)` | Convert to double | `CDBL(42)` | `42` |
| `CSNG(x)` | Convert to single | `CSNG(3.14)` | `3.14` |

*Note: CDBL and CSNG are no-ops since all numbers are stored as doubles*

---

## Function Behavior Comparison

### INT vs FIX vs CINT

These three functions handle numbers differently:

| Input | INT | FIX | CINT | Notes |
|-------|-----|-----|------|-------|
| `3.2` | `3` | `3` | `3` | All same for positive |
| `3.5` | `3` | `3` | `4` | CINT rounds |
| `3.7` | `3` | `3` | `4` | CINT rounds |
| `-3.2` | `-4` | `-3` | `-3` | INT floors down |
| `-3.5` | `-4` | `-3` | `-4` | Different behaviors |
| `-3.7` | `-4` | `-3` | `-4` | FIX truncates |

**Summary:**
- **INT(x)**: Always rounds DOWN (towards -infinity)
- **FIX(x)**: Always truncates towards ZERO
- **CINT(x)**: Rounds to NEAREST integer (0.5 rounds up)

### Example Program
```basic
10 PRINT "INT(3.7) = "; INT(3.7)    ' 3
20 PRINT "FIX(3.7) = "; FIX(3.7)    ' 3
30 PRINT "CINT(3.7) = "; CINT(3.7)  ' 4
40 PRINT ""
50 PRINT "INT(-3.7) = "; INT(-3.7)   ' -4
60 PRINT "FIX(-3.7) = "; FIX(-3.7)   ' -3
70 PRINT "CINT(-3.7) = "; CINT(-3.7) ' -4
```

---

## Trigonometric Functions

### Important: Uses Radians

All trigonometric functions use **radians**, not degrees.

**To convert degrees to radians:**
```basic
10 LET PI = 3.14159265
20 LET DEGREES = 45
30 LET RADIANS = DEGREES * PI / 180
40 PRINT "SIN(45°) = "; SIN(RADIANS)
```

### Common Values

| Angle | Radians | SIN | COS | TAN |
|-------|---------|-----|-----|-----|
| 0° | 0 | 0 | 1 | 0 |
| 30° | π/6 ≈ 0.524 | 0.5 | 0.866 | 0.577 |
| 45° | π/4 ≈ 0.785 | 0.707 | 0.707 | 1 |
| 60° | π/3 ≈ 1.047 | 0.866 | 0.5 | 1.732 |
| 90° | π/2 ≈ 1.571 | 1 | 0 | ∞ |
| 180° | π ≈ 3.142 | 0 | -1 | 0 |

### ATN (Arctangent)

Returns the angle in radians whose tangent is x.

```basic
10 PRINT "ATN(1) = "; ATN(1)  ' π/4 ≈ 0.7854
20 PRINT "ATN(0) = "; ATN(0)  ' 0
```

**To get angle in degrees:**
```basic
10 LET PI = 3.14159265
20 LET RADIANS = ATN(1)
30 LET DEGREES = RADIANS * 180 / PI
40 PRINT "Angle: "; DEGREES; " degrees"  ' 45
```

---

## Random Numbers

### RND and RANDOM

Both generate random numbers between 0 and 1.

```basic
10 PRINT RND(0)     ' e.g., 0.7234
20 PRINT RANDOM(0)  ' e.g., 0.4521
```

**To get random integer from 1 to N:**
```basic
10 LET N = 10
20 LET R = INT(RND(0) * N) + 1
30 PRINT "Random 1-10: "; R
```

**To get random integer from A to B:**
```basic
10 LET A = 5
20 LET B = 15
30 LET R = INT(RND(0) * (B - A + 1)) + A
40 PRINT "Random 5-15: "; R
```

---

## Exponential and Logarithmic

### EXP(x)

Returns e^x (where e ≈ 2.718281828)

```basic
10 PRINT EXP(0)  ' 1
20 PRINT EXP(1)  ' 2.718281828
30 PRINT EXP(2)  ' 7.389056099
```

### LOG(x)

Returns logarithm base 10 of x.

```basic
10 PRINT LOG(1)    ' 0
20 PRINT LOG(10)   ' 1
30 PRINT LOG(100)  ' 2
40 PRINT LOG(1000) ' 3
```

**For natural logarithm (ln):**
```basic
10 REM Natural log = LOG(X) / LOG(e)
20 LET X = 10
30 LET LN = LOG(X) / LOG(EXP(1))
40 PRINT "LN("; X; ") = "; LN
```

---

## Complex Examples

### Distance Between Points
```basic
10 LET X1 = 0: LET Y1 = 0
20 LET X2 = 3: LET Y2 = 4
30 LET D = SQR((X2-X1)*(X2-X1) + (Y2-Y1)*(Y2-Y1))
40 PRINT "Distance: "; D  ' 5
```

### Angle Between Points
```basic
10 LET X1 = 0: LET Y1 = 0
20 LET X2 = 3: LET Y2 = 3
30 LET ANGLE = ATN((Y2-Y1)/(X2-X1))
40 LET PI = 3.14159265
50 LET DEGREES = ANGLE * 180 / PI
60 PRINT "Angle: "; DEGREES; " degrees"  ' 45
```

### Rounding to N Decimal Places
```basic
10 LET X = 3.14159
20 LET N = 2  REM 2 decimal places
30 LET R = INT(X * 10^N + 0.5) / 10^N
40 PRINT "Rounded: "; R  ' 3.14
```

---

## Test Program

Run `tests/test_math_functions.bas` to verify all functions:

```bash
./build/bin/basic-trs80 tests/test_math_functions.bas
```

**Expected output**: All functions should return values close to expected results.

---

## Compatibility

### TRS-80 Model 3/4 Disk BASIC

✅ **Fully Compatible** with all standard math functions:
- ABS, INT, FIX, CINT, SGN, SQR
- SIN, COS, TAN, ATN
- EXP, LOG
- RND, RANDOM
- CDBL, CSNG

### Microsoft BASIC Variants

✅ **Compatible** with most Microsoft BASIC implementations
- IBM BASIC, GW-BASIC, QuickBASIC
- Commodore BASIC V2+ (with extensions)

### Differences from Some BASICs

⚠️ **LOG is base 10**, not natural log (ln)
- Some BASICs use LOG for natural logarithm
- TRS-80 uses LOG for base 10

⚠️ **Trigonometric functions use radians**
- Some BASICs have both DEG and RAD modes
- TRS-80 always uses radians

---

## Implementation Notes

- All numbers are stored as **IEEE 754 double precision** (64-bit)
- CDBL and CSNG are no-ops (no conversion needed)
- Random numbers use C library `rand()` function
- Trigonometric functions use C library `math.h`
- LOG uses `log10()` (base 10), not `log()` (natural)

---

**Documentation Version**: 1.0  
**Last Updated**: 2026-01-19  
**Interpreter Version**: 1.4.0
