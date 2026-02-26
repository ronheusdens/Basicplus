#!/bin/bash
# Launcher script for TRS-80 Level II BASIC interpreter
# Opens in a new Terminal window with 64x16 size

# Get the directory where this script is located (Contents/MacOS)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BASIC_BIN="${SCRIPT_DIR}/basic-trs80-ast"

# Check if binary exists
if [ ! -f "$BASIC_BIN" ]; then
    osascript -e 'display dialog "BASIC interpreter not found. Please rebuild the application." buttons {"OK"} default button "OK" with icon stop'
    exit 1
fi

# Make sure it's executable
chmod +x "$BASIC_BIN"

# Get absolute path
BASIC_BIN_ABS="$(cd "$(dirname "$BASIC_BIN")" && pwd)/$(basename "$BASIC_BIN")"

# Calculate window bounds for 64x16 characters (approximate)
WINDOW_WIDTH=$((64 * 8 + 50))
WINDOW_HEIGHT=$((16 * 16 + 50))
LEFT=100
TOP=100
RIGHT=$((LEFT + WINDOW_WIDTH))
BOTTOM=$((TOP + WINDOW_HEIGHT))

# Launch in Terminal.app
osascript <<EOF
tell application "Terminal"
    activate
    set newTab to do script "'$BASIC_BIN_ABS'"
    delay 0.5
    set frontWindow to front window
    set bounds of frontWindow to {$LEFT, $TOP, $RIGHT, $BOTTOM}
    set font size of frontWindow to 12
    set font name of frontWindow to "Courier"
    set background color of frontWindow to {0, 0, 0}
    set normal text color of frontWindow to {65535, 65535, 65535}
end tell
EOF
