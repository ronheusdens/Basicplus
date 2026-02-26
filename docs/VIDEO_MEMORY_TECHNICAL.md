# Video Memory System (Technical Detail)

**Date**: February 8, 2026  
**Scope**: AST interpreter video RAM (80x24)  
**Audience**: Maintenance and future development

---

## Overview

The AST interpreter includes an emulated, memory-mapped video RAM that behaves like TRS-80 Level II hardware. The display is a flat 1920-byte buffer, addressable via PEEK/POKE. The buffer is synchronized with screen output, and POKE updates are rendered immediately to the display.

---

## Memory Map

Video RAM is mapped as:

$$
\text{addr} = 0x3C00 + (row \times 80) + col
$$

- Base address: **0x3C00** (15360 decimal)
- End address: **0x437F** (17279 decimal)
- Size: **1920 bytes** (80 columns x 24 rows)
- Row 0, col 0 maps to 0x3C00

Examples:

- (row 0, col 0) -> 15360 (0x3C00)
- (row 1, col 0) -> 15440 (0x3C50)
- (row 5, col 10) -> 15754 (0x3D8A)
- (row 23, col 79) -> 17279 (0x437F)

---

## Core Module: videomem.c / videomem.h

Files:
- [src/ast-modules/videomem.c](src/ast-modules/videomem.c)
- [src/ast-modules/videomem.h](src/ast-modules/videomem.h)

Key components:

- Static storage:
  - `static unsigned char video_ram[VIDEO_SIZE];`

- Constants:
  - `VIDEO_BASE` = 0x3C00
  - `VIDEO_COLS` = 80
  - `VIDEO_ROWS` = 24
  - `VIDEO_SIZE` = 1920
  - `VIDEO_END` = 0x437F

### API

- `videomem_init()`
  - Clears video RAM to spaces (0x20).

- `videomem_peek(address)`
  - Validates range (0x3C00..0x437F).
  - Returns byte value or -1 on error.

- `videomem_poke(address, value)`
  - Validates address and value (0..255).
  - Writes on success, returns -1 on error.

- `videomem_write_char(row, col, ch)`
  - Writes by row/col (bounds checked).

- `videomem_get_char(row, col)`
  - Returns character for row/col or space if out-of-bounds.

- `videomem_scroll_up()`
  - TRS-80 style scroll:
    - `memmove(video_ram, video_ram + 80, 80 * 23)`
    - `memset(video_ram + 80 * 23, 0x20, 80)`

- `videomem_clear()`
  - Clears RAM to spaces (visual display unchanged unless explicitly redrawn).

---

## Runtime Integration (PEEK/POKE)

File:
- [src/ast-modules/runtime.c](src/ast-modules/runtime.c#L972-L1021)

### runtime_poke()

- If address in VRAM range, route to `videomem_poke()`.
- Otherwise writes into the regular runtime memory buffer.

### runtime_peek()

- If address in VRAM range, route to `videomem_peek()`.
- Otherwise reads from regular runtime memory.
- Out-of-range VRAM returns 0 (no error).

This keeps PEEK/POKE semantics uniform across system and video RAM.

---

## POKE Statement Execution

File:
- [src/ast-modules/executor.c](src/ast-modules/executor.c#L1725-L1760)

### Value Handling

The POKE value supports both numeric and string expressions:

- If expression is string (literal, `$` variable, concatenation):
  - Use the first character.
  - Empty string -> 0.

- Otherwise evaluate as numeric:
  - `value = (int)eval_numeric_expr()`

### VRAM writes

If address is in video RAM:

1) `videomem_poke()` updates the buffer
2) A direct render is triggered via `termio_put_char_at(row, col, value)`

### Error handling

- Illegal VRAM access sets Error 200:
  - `runtime_set_error(ctx->runtime, 200, stmt->line_number)`

---

## Rendering Path (SDL)

File:
- [src/ast-modules/termio_sdl.c](src/ast-modules/termio_sdl.c)

### PRINT output

`termio_write_char()` does two things:

- Updates SDL scrollback buffer (`put_char_at()`)
- Updates video RAM via `videomem_write_char()`

This keeps PRINT and VRAM in sync.

### Scrolling

When PRINT hits the bottom row:

- SDL scrollback advances
- `videomem_scroll_up()` shifts VRAM contents

### POKE direct updates

`termio_put_char_at(row, col, c)` renders a single character at a specific row/col without altering cursor state or scrollback position. This is used for POKE, so the screen updates immediately.

---

## Rendering Path (stdio)

File:
- [src/ast-modules/termio.c](src/ast-modules/termio.c#L26-L38)

`termio_put_char_at()` uses ANSI save/restore cursor sequences:

```
\033[s  save cursor
\033[row;colH  move
c  write
\033[u  restore
```

This avoids disturbing the prompt position.

---

## CLS Behavior

CLS clears the display but does not clear video RAM.

- In SDL backend: `termio_clear()` resets the screen buffer only.
- VRAM stays intact until overwritten by PRINT/POKE.

This mirrors your requirement that CLS is visual-only.

---

## Browse Highlight Toggle (SDL)

A highlight bar is optional:

- Default: off
- Enable via environment variable:
  - `TRS80_BROWSE_HIGHLIGHT=1`
- Toggle at runtime using `F2`

Key logic is in:
- [src/ast-modules/termio_sdl.c](src/ast-modules/termio_sdl.c#L170-L210)
- Input event handling in both `termio_readline()` and `termio_lineedit()`

---

## Maintenance Notes

### If you change screen dimensions

Update these constants together:

- `VIDEO_COLS`, `VIDEO_ROWS` in videomem.h
- `TERM_COLS`, `TERM_ROWS` in termio_sdl.c
- Any row/col calculations in executor and termio

### If you want attributes or color

The current VRAM is 1 byte per cell. You could expand to 2 bytes per cell:

- Byte 0: character
- Byte 1: attributes

This requires:

- Doubling video RAM size
- Updating `videomem_*()` calculations
- Updating render loop to apply attributes

### If you want redraw from VRAM

Currently PRINT/POKE update both VRAM and SDL directly. An alternative is:

1) Update VRAM only
2) In render loop, redraw from VRAM each frame

This requires a direct VRAM-backed render pass in `render_text()`.

---

## Key Files

- [src/ast-modules/videomem.c](src/ast-modules/videomem.c)
- [src/ast-modules/videomem.h](src/ast-modules/videomem.h)
- [src/ast-modules/runtime.c](src/ast-modules/runtime.c#L972-L1021)
- [src/ast-modules/executor.c](src/ast-modules/executor.c#L1725-L1760)
- [src/ast-modules/termio_sdl.c](src/ast-modules/termio_sdl.c)
- [src/ast-modules/termio.c](src/ast-modules/termio.c#L26-L38)
- [src/ast-modules/termio.h](src/ast-modules/termio.h#L9-L16)

---

## Summary

The video memory system is a clean, modular layer that:

- Emulates TRS-80 style memory-mapped display
- Keeps PRINT and PEEK/POKE in sync
- Supports immediate visual updates
- Preserves legacy semantics (CLS visual only)

This design allows future extensions (attributes, color, full VRAM redraw) without changing the BASIC language surface.
