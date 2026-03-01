#!/bin/bash
#
# BasicPP Interpreter Runner for VIM
# Usage: basic++-run.sh [filename]
# Or from vim with \p: pipes current buffer to this script
#
# This script:
# 1. Takes a .basicpp file as input (piped or as argument)
# 2. Executes it with the basicpp interpreter
# 3. Shows output inline
# 4. Drops to interactive shell (type 'exit' to return to VIM)
#

# Configuration
BASICPP_PATH="/Users/ronheusdens/Stack/Dev/MacSilicon/c/basic++/build/bin/basicpp"

# Determine input mode: piped stdin vs file argument
USE_PIPED_INPUT=false
TEMP_INPUT_FILE=""
INPUT_FILE=""

if [ ! -t 0 ]; then
    # Stdin is piped (from VIM with :read !basic++-run.sh %)
    USE_PIPED_INPUT=true
    TEMP_INPUT_FILE="/tmp/vim_basicpp_$$.basicpp"
    cat > "$TEMP_INPUT_FILE"
elif [ -z "$1" ]; then
    echo "Error: No input file provided" >&2
    exit 1
else
    INPUT_FILE="$1"
fi

# Validate interpreter exists
if [ ! -f "$BASICPP_PATH" ]; then
    echo "Error: Interpreter not found at $BASICPP_PATH" >&2
    echo "Please build it first with: cd /Users/ronheusdens/Stack/Dev/MacSilicon/c/basic++ && make build" >&2
    exit 1
fi

# For file-based input, convert to absolute path
if [ "$USE_PIPED_INPUT" = false ]; then
    # Validate input file exists
    if [ ! -f "$INPUT_FILE" ]; then
        echo "Error: Input file not found: $INPUT_FILE" >&2
        exit 1
    fi
    
    # Convert to absolute path
    if [[ ! "$INPUT_FILE" = /* ]]; then
        INPUT_FILE="$(cd "$(dirname "$INPUT_FILE")" 2>/dev/null && pwd)/$(basename "$INPUT_FILE")" || INPUT_FILE="$1"
    fi
fi

# Run the interpreter
RUN_FILE="${TEMP_INPUT_FILE:-$INPUT_FILE}"

echo "=========================================="
echo "Running BasicPP: $RUN_FILE"
echo "=========================================="
"$BASICPP_PATH" "$RUN_FILE"
EXIT_CODE=$?

# Clean up temp file if it was created
if [ -n "$TEMP_INPUT_FILE" ] && [ -f "$TEMP_INPUT_FILE" ]; then
    rm -f "$TEMP_INPUT_FILE"
fi

# Drop to shell for interaction
echo "=========================================="
echo "BasicPP execution completed (exit code: $EXIT_CODE)"
echo "Type 'exit' to return to VIM"
echo "=========================================="
"$SHELL" 2>/dev/null
