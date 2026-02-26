# LOG() Function

## Overview
Returns the base-10 logarithm of a number.

## Syntax
```basic
result = LOG(x)
```

## Parameters
- **x**: Positive number

## Return Value
- Base-10 logarithm of x
- Returns 0 if x ≤ 0

## Examples
```basic
10 PRINT LOG(100)      ' Output: 2
20 PRINT LOG(1)        ' Output: 0
30 PRINT LOG(0)        ' Output: 0
```

## Notes
- Only defined for positive x
- LOG(0) and LOG(negative) return 0
- For natural logarithm, use LOG(x) / LOG(2.71828)

## Related Functions
- [EXP()](EXP.md)
- [SQR()](SQR.md)
