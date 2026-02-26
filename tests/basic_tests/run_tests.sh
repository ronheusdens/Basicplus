#!/usr/bin/env bash
set -euo pipefail

# Usage: ./run_tests.sh [path-to-basic-binary] [test-filter]
#   test-filter: optional; if set (e.g. 49), run only NN*.bas matching that prefix
BASIC_BIN="${1:-../basic-trs80-ast}"
TEST_FILTER="${2:-}"

if [[ ! -x "$BASIC_BIN" ]]; then
  echo "basic binary not found/executable: $BASIC_BIN" >&2
  exit 1
fi

# Script directory (where .bas and .out files live); must be set before resolving BASIC_BIN
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Convert to absolute path (this may change cwd)
BASIC_BIN="$(cd "$(dirname "$BASIC_BIN")" && pwd)/$(basename "$BASIC_BIN")"

cd "$SCRIPT_DIR"

if [[ -n "$TEST_FILTER" ]]; then
  shopt -s nullglob
  BAS_FILES=("${TEST_FILTER}"*.bas)
  shopt -u nullglob
  if [[ ${#BAS_FILES[@]} -eq 0 ]]; then
    echo "No tests match: ${TEST_FILTER}*.bas" >&2
    exit 1
  fi
else
  BAS_FILES=(*.bas)
fi

total=0
pass=0

for bas in "${BAS_FILES[@]}"; do
  total=$((total+1))
  base="${bas%.bas}"
  exp="${base}.out"
  if [[ ! -f "$exp" ]]; then
    echo "Missing expected output: $exp" >&2
    exit 1
  fi

  got="$(mktemp)"
  "$BASIC_BIN" "$SCRIPT_DIR/$bas" >"$got" 2>&1 || true

  # Trim trailing spaces (not tabs) for stable diffs
  # Use perl instead of sed to handle binary data correctly
  perl -i -pe 's/ +$//' "$got"

  if diff -u "$exp" "$got" >/dev/null; then
    echo "PASS $bas" >&2
    pass=$((pass+1))
  else
    echo "FAIL $bas" >&2
    diff -u "$exp" "$got" || true
  fi
  rm -f "$got"
done

echo "$pass/$total tests passed" >&2
[[ $pass -eq $total ]]
