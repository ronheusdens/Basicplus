#!/bin/bash
# BasicPP app launcher - runs basicpp with passed file argument via Terminal
DIR="$( cd "$( dirname "$0" )" && pwd )"
BINARY="$DIR/basicpp"

# Receive the file path as first argument
FILE_ARG="$1"

# Build the command to run
if [ -n "$FILE_ARG" ]; then
    CMD="\"$BINARY\" \"$FILE_ARG\""
else
    CMD="\"$BINARY\""
fi

# Open Terminal with the command
osascript <<EOF
tell application "Terminal"
    activate
    do script "$CMD"
end tell
EOF


