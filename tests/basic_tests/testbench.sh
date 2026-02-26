#!/bin/bash

# Ensure we're in the test directory
cd "$(dirname "$0")" || exit 1

BINARY="/Users/ronheusdens/Stack/Dev/MacSilicon/c/basic/build/bin/basic-trs80-ast"

if [[ ! -x "$BINARY" ]]; then
    echo "ERROR: Could not find basic-trs80-ast binary at $BINARY"
    exit 1
fi

OUTPUT_FILE="testbench_output.txt"
> "$OUTPUT_FILE"

total=0
pass=0

for bas in [0-9][0-9]_*.bas; do
    [[ -f "$bas" ]] || continue
    
    total=$((total+1))
    base="${bas%.bas}"
    exp_file="${base}.out"
    
    if [[ ! -f "$exp_file" ]]; then
        echo "ERROR: Expected output file not found: $exp_file" >&2
        echo "FAIL $bas (missing $exp_file)" | tee -a "$OUTPUT_FILE"
        continue
    fi
    
    got=$(mktemp)
    
    # Run test with absolute path
    "$BINARY" "$(pwd)/$bas" > "$got" 2>&1
    exit_code=$?
    
    if [[ $exit_code -eq 0 ]]; then
        LC_ALL=C sed -i '' -e 's/ *$//' "$got"
        
        if diff -q "$exp_file" "$got" > /dev/null 2>&1; then
            echo "PASS $bas" | tee -a "$OUTPUT_FILE"
            pass=$((pass+1))
        else
            echo "FAIL $bas (output mismatch)" | tee -a "$OUTPUT_FILE"
        fi
    else
        echo "FAIL $bas (exit code: $exit_code)" | tee -a "$OUTPUT_FILE"
    fi
    
    rm -f "$got"
done

echo "---" | tee -a "$OUTPUT_FILE"
echo "Total: $pass/$total tests passed" | tee -a "$OUTPUT_FILE"

if [[ $pass -eq $total ]]; then
    exit 0
else
    exit 1
fi

