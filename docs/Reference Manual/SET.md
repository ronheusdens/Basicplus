# SET Statement

## Overview
Sets (draws) a pixel at the specified coordinates in graphics mode.

## Syntax
```basic
SET x, y
SET x, y, color
```

## Parameters
- **x**: X coordinate (integer)
- **y**: Y coordinate (integer)
- **color** (optional): Color code (0–7). If omitted, uses current color.

## Description
- Draws a pixel at (x, y) in the current graphics screen.
- If color is omitted, uses the current foreground color (set by COLOR statement).
- Only works in graphics mode (SCREEN 1).

## Examples
```basic
10 SCREEN 1
20 COLOR 2
30 SET 10, 20        ' Green pixel at (10,20)
40 SET 15, 25, 4      ' Red pixel at (15,25)
50 END
```

## Notes
- Coordinates outside the screen are ignored.
- Color code must be in the valid range (0–7).
- SET does nothing in text mode (SCREEN 0).

## Related Statements
- [RESET](RESET.md) - Clears a pixel
- [COLOR](COLOR.md) - Set current color
- [SCREEN](SCREEN.md) - Set graphics/text mode
