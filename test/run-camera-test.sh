#!/bin/sh
set -eu

test_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
config=${1:-"$test_dir/raw-1080p30.conf"}
if [ ! -r "$config" ]; then
    echo "Configuration not found: $config" >&2
    exit 1
fi
. "$config"
: "${FRAME_COUNT:?Missing FRAME_COUNT}"
: "${OUTPUT_DIR:?Missing OUTPUT_DIR}"
: "${SAVE_FRAME:?Missing SAVE_FRAME}"
case "$FRAME_COUNT" in ''|*[!0-9]*) echo "Invalid FRAME_COUNT" >&2; exit 1;; esac
if [ "$FRAME_COUNT" -lt 2 ]; then
    echo "FRAME_COUNT must be at least 2" >&2
    exit 1
fi
case "$SAVE_FRAME" in 0|1) ;; *) echo "SAVE_FRAME must be 0 or 1" >&2; exit 1;; esac
if [ ! -c /dev/video0 ]; then
    echo "/dev/video0 is absent; check GC2093 driver initialization first." >&2
    exit 1
fi
if [ ! -x "$test_dir/gc2093_raw_test" ]; then
    echo "Build gc2093_raw_test on the SDK host, then copy the test directory to the board." >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"
echo "1080P30 RAW test: $FRAME_COUNT frames; output: $OUTPUT_DIR"
result=0
if [ "$SAVE_FRAME" = 1 ]; then
    "$test_dir/gc2093_raw_test" "$FRAME_COUNT" "$OUTPUT_DIR/frame.raw" > "$OUTPUT_DIR/capture.log" 2>&1 || result=$?
else
    "$test_dir/gc2093_raw_test" "$FRAME_COUNT" > "$OUTPUT_DIR/capture.log" 2>&1 || result=$?
fi
printf '%s\n' "$result" > "$OUTPUT_DIR/exit-code.txt"
cat "$OUTPUT_DIR/capture.log"
sync
echo "Capture exit code: $result"
exit "$result"
