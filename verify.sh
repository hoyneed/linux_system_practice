#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

TMPDIR_TEST="$(mktemp -d)"
DAEMON_PID=""
FW_PID=""

cleanup() {
    if [[ -n "$DAEMON_PID" ]] && kill -0 "$DAEMON_PID" 2>/dev/null; then
        kill -9 "$DAEMON_PID" 2>/dev/null || true
        wait "$DAEMON_PID" 2>/dev/null || true
    fi
    if [[ -n "$FW_PID" ]] && kill -0 "$FW_PID" 2>/dev/null; then
        kill -9 "$FW_PID" 2>/dev/null || true
        wait "$FW_PID" 2>/dev/null || true
    fi
    rm -rf "$TMPDIR_TEST"
}
trap cleanup EXIT

echo "=== [Test 1] Clean Build & Zero Warnings Check ==="
make clean >"$TMPDIR_TEST/build.log" 2>&1
make all practice >>"$TMPDIR_TEST/build.log" 2>&1
cat "$TMPDIR_TEST/build.log"
if grep -qiE "warning:|error:" "$TMPDIR_TEST/build.log"; then
    echo "FAIL: Compiler warnings or errors detected!"
    exit 1
fi
echo "PASS: Build succeeded with 0 warnings and 0 errors."

echo ""
echo "=== [Test 2] 01_signal_shell (miner_daemon + miner_ctl) ==="
if ./01_signal_shell/miner_ctl >/dev/null 2>&1; then
    echo "FAIL: miner_ctl should fail when PID argument is missing."
    exit 1
fi
if ./01_signal_shell/miner_ctl abc >/dev/null 2>&1; then
    echo "FAIL: miner_ctl should fail on non-numeric PID."
    exit 1
fi
if ./01_signal_shell/miner_ctl 123abc >/dev/null 2>&1; then
    echo "FAIL: miner_ctl should fail on mixed alphanumeric PID (strtol check)."
    exit 1
fi
if ./01_signal_shell/miner_ctl -5 >/dev/null 2>&1; then
    echo "FAIL: miner_ctl should fail on negative PID."
    exit 1
fi

./01_signal_shell/miner_daemon >"$TMPDIR_TEST/daemon.log" 2>&1 &
DAEMON_PID=$!
sleep 0.25

if ! kill -0 "$DAEMON_PID" 2>/dev/null; then
    echo "FAIL: miner_daemon failed to stay running."
    cat "$TMPDIR_TEST/daemon.log"
    exit 1
fi

PGREP_PIDS="$(pgrep -x miner_daemon || true)"
PIDOF_PIDS="$(pidof miner_daemon || true)"
echo "Verified PID lookup -> DAEMON_PID=$DAEMON_PID, pgrep=[$PGREP_PIDS], pidof=[$PIDOF_PIDS]"
if [[ "$PGREP_PIDS" != *"$DAEMON_PID"* ]] || [[ "$PIDOF_PIDS" != *"$DAEMON_PID"* ]]; then
    echo "FAIL: pgrep/pidof did not match DAEMON_PID=$DAEMON_PID"
    exit 1
fi

(
    printf 'add\n'
    sleep 0.15
    printf 'show\n'
    sleep 0.15
    printf 'del\n'
    sleep 0.15
    printf 'invalid_cmd\n'
    sleep 0.15
    printf 'stop\n'
    sleep 0.15
    printf 'exit\n'
) | ./01_signal_shell/miner_ctl "$DAEMON_PID" >"$TMPDIR_TEST/ctl.log" 2>&1

wait "$DAEMON_PID"
DAEMON_PID=""

grep -q "Sent SIGUSR1" "$TMPDIR_TEST/ctl.log"
grep -q "Sent SIGINT" "$TMPDIR_TEST/ctl.log"
grep -q "Sent SIGUSR2" "$TMPDIR_TEST/ctl.log"
grep -q "Unknown command: 'invalid_cmd'" "$TMPDIR_TEST/ctl.log"
grep -q "Sent SIGTERM" "$TMPDIR_TEST/ctl.log"
grep -q "Bye!" "$TMPDIR_TEST/ctl.log"

grep -q "SIGUSR1 received -> Miner added" "$TMPDIR_TEST/daemon.log"
grep -q "SIGINT received -> Coins:" "$TMPDIR_TEST/daemon.log"
grep -q "SIGUSR2 received -> Miner removed" "$TMPDIR_TEST/daemon.log"
grep -q "SIGTERM received -> Shutting down" "$TMPDIR_TEST/daemon.log"
echo "PASS: 01_signal_shell passed all checks."

echo ""
echo "=== [Test 3] 02_thread_miner (miner_thread) ==="
(
    printf 'add\n'
    sleep 0.35
    printf 'show\n'
    sleep 0.1
    printf 'del\n'
    sleep 0.1
    printf 'unknown\n'
    sleep 0.1
    printf 'exit\n'
) | ./02_thread_miner/miner_thread >"$TMPDIR_TEST/thread.log" 2>&1

grep -q "Miner #2 added" "$TMPDIR_TEST/thread.log"
grep -q "Oldest Miner #1 removed" "$TMPDIR_TEST/thread.log"
grep -q "Unknown command: 'unknown'" "$TMPDIR_TEST/thread.log"
grep -q "Shutting down miners" "$TMPDIR_TEST/thread.log"

COINS_VAL="$(sed -n 's/.*\[SHOW\] Coins: \([0-9][0-9]*\).*/\1/p' "$TMPDIR_TEST/thread.log" | head -n 1)"
if [[ -z "$COINS_VAL" ]] || [[ "$COINS_VAL" -le 0 ]]; then
    echo "FAIL: Expected positive mined coins, got '$COINS_VAL'"
    cat "$TMPDIR_TEST/thread.log"
    exit 1
fi
echo "PASS: 02_thread_miner passed (show reported Coins: $COINS_VAL)."

echo ""
echo "=== [Test 4] 03_firmware_person (firmware + person) ==="
./03_firmware_person/firmware >"$TMPDIR_TEST/fw.log" 2>&1 &
FW_PID=$!
sleep 0.25

(
    printf '1\n'
    sleep 0.15
    printf '2\n'
    sleep 0.15
    printf 'bad\n'
    sleep 0.15
    printf '3\n'
    sleep 0.15
    printf 'exit\n'
) | ./03_firmware_person/person "$FW_PID" >"$TMPDIR_TEST/person.log" 2>&1

wait "$FW_PID"
FW_PID=""

grep -q "Sent SIGUSR1" "$TMPDIR_TEST/person.log"
grep -q "Sent SIGUSR2" "$TMPDIR_TEST/person.log"
grep -q "Invalid input 'bad'" "$TMPDIR_TEST/person.log"
grep -q "Sent SIGTERM" "$TMPDIR_TEST/person.log"
grep -q "Person App exiting" "$TMPDIR_TEST/person.log"

grep -q "\[Button Clicked\]" "$TMPDIR_TEST/fw.log"
grep -q "RESET" "$TMPDIR_TEST/fw.log"
grep -q "BYE" "$TMPDIR_TEST/fw.log"
echo "PASS: 03_firmware_person passed all checks."

echo ""
echo "ALL TESTS PASSED!"
