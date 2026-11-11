#!/bin/sh
# Stop the child owned by run.sh; its parent restores the previous ISP override.
set -eu
LOCK=/tmp/ov13850-display.lock
if [ ! -f "$LOCK/pid" ]; then echo 'No active display runner.'; exit 0; fi
PID=$(cat "$LOCK/pid")
case "$PID" in ''|*[!0-9]*) echo 'Invalid PID file' >&2; exit 1;; esac
tr '\000' '\n' < "/proc/$PID/cmdline" | head -n 1 | grep -q '/app/sample_smartIPC_demo$' || exit 1
kill -INT "$PID"
