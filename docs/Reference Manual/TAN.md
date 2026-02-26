# TAN() Function

## Overview
Returns the tangent of an angle (in radians).

## Syntax
```basic
result = TAN(x)
```

## Parameters
- **x**: Angle in radians

## Return Value
- Tangent of the angle

## Examples
```basic
10 PRINT TAN(0)        ' Output: 0
20 PRINT TAN(3.14159/4) ' Output: ~1
```

## Notes
- Argument is in radians, not degrees
- To convert degrees to radians: radians = degrees * 3.14159 / 180
- May overflow for odd multiples of PI/2

## Related Functions
- [SIN()](SIN.md)
- [COS()](COS.md)
- [ATN()](ATN.md)
