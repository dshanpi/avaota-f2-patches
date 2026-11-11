#!/bin/sh
# Run only one OV13850 demo and restore the previous global ISP override on exit.
set -eu
BASE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
MODE=${1:-1440p30}
case "$MODE" in 1080p30|1440p30) FPS=30;; 4k25) FPS=25;; *) echo 'Usage: sh run.sh 1080p30|1440p30|4k25' >&2; exit 2;; esac
NET_TYPE=${NET_TYPE:-3}
case "$NET_TYPE" in 0|3) ;; *) echo 'NET_TYPE must be 3 (wlan0) or 0 (loopback/ADB forward)' >&2; exit 2;; esac
DURATION=${DURATION:-0}
case "$DURATION" in ''|*[!0-9]*) echo 'DURATION must be seconds, 0 means continuous' >&2; exit 2;; esac
APP="$BASE/app/sample_smartIPC_demo"
LIB="$BASE/lib/librtsp_tcp_writeall.so"
IQ=/tmp/isp_test/color/isp_param_config.bin
LOCK=/tmp/ov13850-display.lock
if pidof sample_smartIPC_demo >/dev/null; then
    echo 'sample_smartIPC_demo is already running. Stop it before switching modes.' >&2
    exit 1
fi
mkdir "$LOCK" 2>/dev/null || { echo "Another runner owns $LOCK; check it before retrying." >&2; exit 1; }
CHILD=
OWNED=0
BACKUP=0
cleanup() {
    trap - EXIT INT TERM
    if [ -n "$CHILD" ] && kill -0 "$CHILD" 2>/dev/null; then
        kill -INT "$CHILD" 2>/dev/null || :
        wait "$CHILD" 2>/dev/null || :
    fi
    if [ "$OWNED" = 1 ]; then
        if [ "$BACKUP" = 1 ]; then cp "$LOCK/original-iq.bin" "$IQ"; else rm -f "$IQ"; fi
    fi
    rm -f "$LOCK/original-iq.bin" "$LOCK/run.conf" "$LOCK/pid"
    rmdir "$LOCK"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
test -f "$APP" && test -f "$LIB" && test -f "$BASE/iq$FPS/isp_param_config.bin"
chmod +x "$APP"
mkdir -p /tmp/isp_test/color
if [ -f "$IQ" ]; then cp "$IQ" "$LOCK/original-iq.bin"; BACKUP=1; fi
OWNED=1
cp "$BASE/iq$FPS/isp_param_config.bin" "$IQ"
sed -e "s/^rtsp_net_type[[:space:]]*=.*/rtsp_net_type = $NET_TYPE/" \
    -e "s/^test_duration[[:space:]]*=.*/test_duration = $DURATION/" \
    "$BASE/profiles/$MODE.conf" > "$LOCK/run.conf"
echo "OV13850 $MODE; network type $NET_TYPE; duration $DURATION s (0=continuous)"
echo 'WiFi player: rtsp://BOARD_WLAN0_IP:8554/ch0 (RTSP over TCP)'
LD_PRELOAD="$LIB" "$APP" -path "$LOCK/run.conf" &
CHILD=$!
echo "$CHILD" > "$LOCK/pid"
set +e
wait "$CHILD"
RESULT=$?
set -e
exit "$RESULT"
