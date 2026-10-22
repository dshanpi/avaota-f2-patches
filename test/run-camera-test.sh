#!/bin/sh
# Run on Avaota F2, using the matching OV13850 firmware and a mounted TF card.
set -u
MODE=${1:-1080p30}
ACTION=${2:-save}
case "$MODE" in
    720p30|720p60|4k25) APP=sample_virvi-modes ;;
    1080p30|1080p60|1440p30|1440p60) APP=sample_virvi-original ;;
    *) echo 'Mode: 720p30, 720p60, 1080p30, 1080p60, 1440p30, 1440p60, 4k25'; exit 2 ;;
esac
case "$ACTION" in save|stream) ;; *) echo 'Action: save or stream'; exit 2 ;; esac
BASE=$(CDPATH= cd "$(dirname "$0")" && pwd) || exit 2
grep -q ' /mnt/SDCARD ' /proc/mounts || { echo 'TF card is not mounted at /mnt/SDCARD'; exit 2; }
if pidof sample_virvi sample_virvi-original sample_virvi-modes >/dev/null 2>&1; then
    echo 'A capture program is already running; stop it before starting another test.'; exit 2
fi
test -r "$BASE/config/$MODE-$ACTION.conf" || exit 2
test -f "$BASE/bin/$APP" || exit 2
chmod +x "$BASE/bin/$APP" || exit 2
RESULT="$BASE/results/$MODE-$ACTION-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$BASE/results" || exit 2
mkdir "$RESULT" || exit 2
sed "s|/mnt/SDCARD/ov13850-multimode-current/frame.nv21|$RESULT/frame.nv21|" \
    "$BASE/config/$MODE-$ACTION.conf" > "$RESULT/capture.conf" || exit 2
cat /proc/sys/kernel/random/boot_id > "$RESULT/boot-id.txt"
dmesg > "$RESULT/dmesg-before.txt"
echo "Mode=$MODE action=$ACTION duration=35s"
echo "Results: $RESULT"
LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib "$BASE/bin/$APP" -path "$RESULT/capture.conf" > "$RESULT/capture.log" 2>&1 &
CAPTURE_PID=$!
interrupted=0
trap 'interrupted=1; kill -INT "$CAPTURE_PID" 2>/dev/null || true' INT TERM
(
    sleep 10
    { cat /proc/uptime; cat /sys/kernel/debug/mpp/vi; cat /proc/uptime; } > "$RESULT/vi-t10.txt"
    cat /sys/kernel/debug/mpp/mipi > "$RESULT/mipi.txt"
    cat /sys/kernel/debug/mpp/isp > "$RESULT/isp.txt"
    sleep 20
    { cat /proc/uptime; cat /sys/kernel/debug/mpp/vi; cat /proc/uptime; } > "$RESULT/vi-t30.txt"
) &
SAMPLER_PID=$!
wait "$CAPTURE_PID"
STATUS=$?
if [ "$interrupted" -eq 1 ]; then
    wait "$CAPTURE_PID" 2>/dev/null || true
    kill "$SAMPLER_PID" 2>/dev/null || true
    STATUS=130
else
    wait "$SAMPLER_PID" || true
fi
trap - INT TERM
printf '%s\n' "$STATUS" > "$RESULT/exit-code.txt"
dmesg > "$RESULT/dmesg-after.txt"
cat /sys/kernel/debug/mpp/vi > "$RESULT/vi-end.txt"
VI=vi0
[ "$MODE" = 4k25 ] && VI=vi1
if [ -s "$RESULT/vi-t10.txt" ] && [ -s "$RESULT/vi-t30.txt" ]; then
    awk -v wanted="$VI:" '
    FNR==1 {file++; first=$1; found=0}
    /^vi[0-9]+:/ {active=($1==wanted)}
    /^frame =>/ && active {gsub(/,/, ""); frames[file]=$4; lost[file]=$6; errors[file]=$8; found=1}
    {last=$1; times[file]=(first+last)/2; valid[file]=found}
    END {
        dt=times[2]-times[1];
        if (file!=2 || !valid[1] || !valid[2] || dt<=0) {print "VI samples unavailable; inspect logs"; exit 1}
        printf "VI window: %.3fs, fps: %.3f, lost delta: %d, error delta: %d\n",dt,(frames[2]-frames[1])/dt,lost[2]-lost[1],errors[2]-errors[1]
    }' "$RESULT/vi-t10.txt" "$RESULT/vi-t30.txt" > "$RESULT/fps.txt" || true
    cat "$RESULT/fps.txt"
fi
if [ -f "$RESULT/frame.nv21" ]; then
    wc -c < "$RESULT/frame.nv21"
    md5sum "$RESULT/frame.nv21" > "$RESULT/frame.md5"
fi
sync
tail -5 "$RESULT/capture.log"
echo "Capture exit=$STATUS; results: $RESULT"
exit "$STATUS"
