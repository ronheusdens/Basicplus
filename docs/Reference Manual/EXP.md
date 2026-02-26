# EXP() Function

## Overview
Returns e (Euler's number) raised to the power of x.

## Syntax
```basic
result = EXP(x)
```

## Parameters
- **x**: Exponent

## Return Value
- e^x (where e ≈ 2.71828)

## Examples
```basic
10 PRINT EXP(1)        ' Output: ~2.71828
20 PRINT EXP(0)        ' Output: 1
30 PRINT EXP(2)        ' Output: ~7.38906
```

## Notes
- EXP(LOG(x)) = x for x > 0
- EXP is the inverse of LOG

## Related Functions
- [LOG()](LOG.md)
- [SQR()](SQR.md)
