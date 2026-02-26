#!/bin/bash
#
# BASIC Interpreter Runner for VIM
# Usage: basic-run.sh [filename]
# Or from vim: :!basic-run.sh %
#
# This script:
# 1. Takes a .bas file as input
# 2. Checks if the interpreter is running
# 3. Kills any running instance
# 4. Loads the .bas file using LOAD command
# 5. Runs it with RUN command
# 6. Keeps interpreter interactive (Ctrl-C returns to prompt)
#
# Uses the AST-based interpreter (basic-trs80-ast)
#

# Configuration
BASE_DIR="/Users/ronheusdens/Stack/Dev/MacSilicon/c/basic/build/bin"
INTERPRETER_PATH="$BASE_DIR/basic-trs80-ast"
INTERPRETER_NAME="basic-trs80-ast"

# Determine input mode: piped stdin vs file
USE_PIPED_INPUT=false
TEMP_INPUT_FILE=""
INPUT_FILE=""

if [ ! -t 0 ]; then
    # Stdin is piped (e.g., from VIM with :w !basic-run.sh)
    USE_PIPED_INPUT=true
    TEMP_INPUT_FILE="/tmp/vim_basic_$$.bas"
    cat > "$TEMP_INPUT_FILE"
elif [ -z "$1" ]; then
    echo "Error: No input file provided" >&2
    exit 1
else
    INPUT_FILE="$1"
fi

# Validate interpreter exists
if [ ! -f "$INTERPRETER_PATH" ]; then
    echo "Error: Interpreter not found at $INTERPRETER_PATH" >&2
    echo "Please build it first with: make basic-trs80-ast" >&2
    exit 1
fi

# For file-based input, convert to absolute path and validate
if [ "$USE_PIPED_INPUT" = false ]; then
    # Validate input file exists
    if [ ! -f "$INPUT_FILE" ]; then
        echo "Error: Input file not found: $INPUT_FILE" >&2
        exit 1
    fi

    # Convert to absolute path for LOAD command
    if command -v realpath >/dev/null 2>&1; then
        INPUT_FILE=$(realpath "$INPUT_FILE")
    elif command -v readlink >/dev/null 2>&1 && readlink -f "$INPUT_FILE" >/dev/null 2>&1; then
        INPUT_FILE=$(readlink -f "$INPUT_FILE")
    else
        # Fallback: manual absolute path conversion
        if [[ "$INPUT_FILE" != /* ]]; then
            if [ -f "$INPUT_FILE" ]; then
                INPUT_FILE="$(cd "$(dirname "$INPUT_FILE")" 2>/dev/null && pwd)/$(basename "$INPUT_FILE")"
            else
                INPUT_FILE="$(pwd)/$INPUT_FILE"
            fi
        fi
        INPUT_FILE=$(cd "$(dirname "$INPUT_FILE")" 2>/dev/null && echo "$(pwd)/$(basename "$INPUT_FILE")")
    fi

    # Verify the absolute path exists
    if [ ! -f "$INPUT_FILE" ]; then
        echo "Error: Could not resolve absolute path for: $1" >&2
        echo "Resolved to: $INPUT_FILE" >&2
        exit 1
    fi
fi

# Kill any running interpreter instances
if pgrep -q "$INTERPRETER_NAME"; then
    echo "Stopping running interpreter..." >&2
    pkill -f "$INTERPRETER_NAME" 2>/dev/null
    sleep 0.2
fi

# Disable SDL for script mode - use text-only stdin/stdout
export BASIC_NO_SDL=1

# For file-based input, set working directory to the file's directory
if [ "$USE_PIPED_INPUT" = false ] && [ -n "$INPUT_FILE" ]; then
    # Get the directory of the input file
    INPUT_DIR=$(cd "$(dirname "$INPUT_FILE")" 2>/dev/null && pwd)
    export BASIC_CWD="$INPUT_DIR"
fi

# Run the interpreter interactively
if [ -c /dev/tty ]; then
    # Interactive mode with FIFO
    FIFO="/tmp/basic_fifo_$$"
    rm -f "$FIFO"
    mkfifo "$FIFO"

    if [ "$USE_PIPED_INPUT" = true ]; then
        # Piped input from VIM: send program lines directly, then RUN
        (
            # Newline for "Memory size?" prompt
            printf '\n'
            # Newline for READY> prompt
            printf '\n'
            # Send each line of the BASIC program
            cat "$TEMP_INPUT_FILE"
            # Run the program
            printf 'RUN\n'
            # After RUN, connect stdin from terminal for interactive use
            cat < /dev/tty
        ) > "$FIFO" &
    else
        # File-based input: use LOAD command
        (
            # Newline for "Memory size?" prompt
            printf '\n'
            # Newline for READY> prompt
            printf '\n'
            printf 'LOAD "%s"\n' "$INPUT_FILE"
            printf 'RUN\n'
            # After RUN, connect stdin from terminal for interactive use
            cat < /dev/tty
        ) > "$FIFO" &
    fi
    
    FEEDER_PID=$!
    "$INTERPRETER_PATH" < "$FIFO"
    # Wait for feeder process to exit naturally (shouldn't take long)
    wait "$FEEDER_PID" 2>/dev/null
    rm -f "$FIFO"
else
    # Non-interactive fallback
    if [ "$USE_PIPED_INPUT" = true ]; then
        (
            printf '\n'
            printf '\n'
            cat "$TEMP_INPUT_FILE"
            printf 'RUN\n'
        ) | "$INTERPRETER_PATH"
    else
        (
            printf '\n'
            printf '\n'
            printf 'LOAD "%s"\n' "$INPUT_FILE"
            printf 'RUN\n'
        ) | "$INTERPRETER_PATH"
    fi
fi

# Clean up temp file if we created one
if [ -n "$TEMP_INPUT_FILE" ] && [ -f "$TEMP_INPUT_FILE" ]; then
    rm -f "$TEMP_INPUT_FILE"
elif [ -z "$1" ] && [ -f "/tmp/vim_basic_$$.bas" ]; then
    rm -f "/tmp/vim_basic_$$.bas"
fi

exit 0
