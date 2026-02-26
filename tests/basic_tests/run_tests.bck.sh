#!/usr/bin/env bash
set -euo pipefail

# Usage: ./run_tests.sh [path-to-basic-binary]
BASIC_BIN="${1:-../basic-trs80-ast}"

if [[ ! -x "$BASIC_BIN" ]]; then
  echo "basic binary not found/executable: $BASIC_BIN" >&2
  exit 1
fi

cd "$(dirname "$0")"

total=0
pass=0

for bas in *.bas; do
  total=$((total+1))
  base="${bas%.bas}"
  exp="${base}.out"
  if [[ ! -f "$exp" ]]; then
    echo "Missing expected output: $exp" >&2
    exit 1
  fi

  got="$(mktemp)"
  "$BASIC_BIN" "$bas" >"$got"

  # Trim trailing spaces (not tabs) for stable diffs
  if sed --version >/dev/null 2>&1; then
    sed -i -e 's/ *$//' "$got"
  else
    sed -i '' -e 's/ *$//' "$got"
  fi

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
