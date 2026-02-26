# WOB and BOW REPL Commands

## Overview

**WOB** (White-On-Black) and **BOW** (Black-On-White) are REPL-only commands that control the terminal display colors in the TRS-80 BASIC interpreter.

## Syntax

```
WOB
BOW
```

Both commands take no arguments.

## Description

### WOB (White-On-Black)

Sets the display to white text on a black background (default). This is the standard TRS-80 BASIC terminal appearance.

**Usage:**
```
> WOB
```

**Display Mode:**
- Text color: White
- Background color: Black
- Edit line (row 24): Dark gray background for visibility

### BOW (Black-On-White)

Sets the display to black text on a white background. Useful for improved contrast or accessibility.

**Usage:**
```
> BOW
```

**Display Mode:**
- Text color: Black
- Background color: White
- Edit line (row 24): Light gray background for visibility

## Examples

```basic
> WOB
> PRINT "WHITE TEXT ON BLACK BACKGROUND"
> BOW
> PRINT "BLACK TEXT ON WHITE BACKGROUND"
> WOB
```

## Notes

- Both commands are **case-insensitive**: `WOB`, `wob`, `Wob` all work
- Commands take effect immediately and apply to all subsequent output
- The commands work with both stdio and SDL2 terminal backends
- The edit line at the bottom (row 24) automatically adjusts its highlighting color for contrast:
  - In WOB mode: dark gray highlight on black background
  - In BOW mode: light gray highlight on white background
- These are REPL commands only; they cannot be used within BASIC program statements

## Platform Support

- **macOS**: Full support (SDL2 rendering)
- **Linux**: Full support (SDL2 rendering)
- **Windows**: Full support (SDL2 rendering)

## Implementation Details

The color state is maintained globally:
- Default: WOB (white foreground, black background)
- Color settings affect all terminal rendering including:
  - Scrollback buffer content
  - Input line
  - Text cursor
  - Edit line preview
