# LN() Function

## Overview
Returns the natural logarithm (base e) of a number.

## Syntax
```basic
result = LN(x)
```

## Parameters
- **x**: Positive number

## Return Value
- Natural logarithm of x (base e)
- Returns 0 if x ≤ 0

## Examples
```basic
10 PRINT LN(1)        ' Output: 0
20 PRINT LN(EXP(1))   ' Output: 1
30 PRINT LN(0)        ' Output: 0
40 PRINT LN(-5)       ' Output: 0
50 PRINT LN(10)       ' Output: ~2.302585
```

## Notes
- Only defined for positive x
- LN(0) and LN(negative) return 0
- For base-10 logarithm, use LOG(x)
- EXP(LN(x)) = x for x > 0

## Related Functions
- [LOG()](LOG.md) - Base-10 logarithm
- [EXP()](EXP.md) - Exponential function
