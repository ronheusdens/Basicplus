# Portable Terminal Interface with ncurses

The BASIC interpreter can be built with ncurses support for a portable terminal interface that works on both macOS and Linux Debian.

## Features

- **64x16 character terminal** - Authentic TRS-80 size
- **Portable** - Works on macOS and Linux
- **Better input handling** - Character-by-character input with backspace support
- **Terminal control** - Proper screen clearing and cursor management
- **Color support** - Black background, white text (if terminal supports colors)

## Building with ncurses

### macOS
```bash
# Install ncurses (if not already installed)
brew install ncurses

# Build with curses support
make basic-trs80-curses
```

### Linux (Debian/Ubuntu)
```bash
# Install ncurses development package
sudo apt-get install libncurses5-dev

# Build with curses support
make basic-trs80-curses
```

## Usage

After building with curses support, run the interpreter:
```bash
./build/bin/basic-trs80
```

The interpreter will:
1. Initialize ncurses terminal
2. Attempt to resize to 64x16 characters
3. Show "Memory size?" prompt
4. Display "RADIO SHACK LEVEL II BASIC" banner
5. Show "READY >" prompt for interactive commands

## Fallback Mode

If ncurses is not available, the interpreter falls back to:
- ANSI escape codes for screen control
- Standard `fgets()` for input
- `printf()` for output

This ensures the interpreter works even without ncurses, though with less precise terminal control.

## Technical Details

- Uses `#ifdef USE_CURSES` for conditional compilation
- Automatically detects ncurses installation paths
- Gracefully falls back to standard I/O if ncurses unavailable
- Proper cleanup on exit via `atexit()`
