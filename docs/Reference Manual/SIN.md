# SIN() Function

## Overview
Returns the sine of an angle (in radians).

## Syntax
```basic
result = SIN(x)
```

## Parameters
- **x**: Angle in radians

## Return Value
- Sine of the angle (range: -1 to 1)

## Examples
```basic
10 PRINT SIN(0)        ' Output: 0
20 PRINT SIN(3.14159/2) ' Output: ~1
30 PRINT SIN(3.14159)   ' Output: ~0
```

## Notes
- Argument is in radians, not degrees
- To convert degrees to radians: radians = degrees * 3.14159 / 180

## Related Functions
- [COS()](COS.md)
- [TAN()](TAN.md)
- [ATN()](ATN.md)
