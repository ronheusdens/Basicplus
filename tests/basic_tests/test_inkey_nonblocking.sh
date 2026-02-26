#!/bin/sh
# Automated INKEY$ non-blocking behavior test
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)
BASIC_BIN="${ROOT}/build/bin/basic-trs80"
PROGRAM="${ROOT}/tests/test_inkey_nonblocking.bas"

if [ ! -x "$BASIC_BIN" ]; then
  echo "basic-trs80 binary not found at $BASIC_BIN" >&2
  exit 1
fi

# Test that INKEY$ completes 100 calls quickly (non-blocking)
OUTPUT=$(perl -e 'alarm 2; exec @ARGV' sh -c "printf '\n' | \"$BASIC_BIN\" \"$PROGRAM\" 2>&1" || echo "TIMEOUT")

if echo "$OUTPUT" | grep "TEST PASSED: INKEY\$ is non-blocking" >/dev/null 2>&1; then
  echo "PASS: INKEY\$ non-blocking test"
  exit 0
elif echo "$OUTPUT" | grep "TIMEOUT" >/dev/null 2>&1; then
  echo "FAIL: INKEY\$ blocked (timeout)" >&2
  exit 1
else
  echo "FAIL: Unexpected output:" >&2
  echo "$OUTPUT" >&2
  exit 1
fi
