# COS() Function

## Overview
Returns the cosine of an angle (in radians).

## Syntax
```basic
result = COS(x)
```

## Parameters
- **x**: Angle in radians

## Return Value
- Cosine of the angle (range: -1 to 1)

## Examples
```basic
10 PRINT COS(0)        ' Output: 1
20 PRINT COS(3.14159/2) ' Output: ~0
30 PRINT COS(3.14159)   ' Output: -1
```

## Notes
- Argument is in radians, not degrees
- To convert degrees to radians: radians = degrees * 3.14159 / 180

## Related Functions
- [SIN()](SIN.md)
- [TAN()](TAN.md)
- [ATN()](ATN.md)
