# SQR() Function

## Overview
Returns the square root of a number.

## Syntax
```basic
result = SQR(x)
```

## Parameters
- **x**: Number (should be ≥ 0)

## Return Value
- Square root of x
- Returns 0 if x < 0

## Examples
```basic
10 PRINT SQR(9)        ' Output: 3
20 PRINT SQR(2)        ' Output: ~1.41421
30 PRINT SQR(-1)       ' Output: 0
```

## Notes
- Only defined for non-negative x
- SQR(0) = 0

## Related Functions
- [EXP()](EXP.md)
- [LOG()](LOG.md)
