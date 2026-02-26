## Terminal I/O, Video Memory, and Platform Notes

This document covers terminal I/O, video memory handling, and platform-specific implementation notes.

### Terminal I/O
- Abstraction via termio.h/termio.c (stdio) and termio_sdl.c (SDL2 windowed).
- Handles input, output, and control characters (e.g., Ctrl-C).

### Video Memory
- 80x24 VRAM at 0x3C00, managed by videomem.c/h.

### Platform Detection
- Makefile detects OS/arch for correct build flags.
- macOS app bundle support (macOS only).

### Memory Management & Type Conversions
- String functions allocate dynamic memory.
- VAL() parses strings as doubles; integer conversion truncates.