#!/usr/bin/env bash
set -euo pipefail
runtime="${1:?private runtime}"
socket="${2:?Wayland display}"
[[ "${CI:-}" == true && "${GITHUB_ACTIONS:-}" == true && -n "${RUNNER_TEMP:-}" &&
   "$runtime" == "$RUNNER_TEMP"/* && "$socket" == wayland-* ]] || exit 2
export XDG_RUNTIME_DIR="$runtime" WAYLAND_DISPLAY="$socket"
mkdir -p "$runtime/home" "$runtime/config-home"
export HOME="$runtime/home" XDG_CONFIG_HOME="$runtime/config-home"
unset DISPLAY
pids=()
cleanup() {
  for p in "${pids[@]}"; do kill -TERM "$p" 2>/dev/null || true; done
  for p in "${pids[@]}"; do wait "$p" 2>/dev/null || true; done
}
trap cleanup EXIT
ipc=""
for _ in {1..60}; do
  for candidate in "$runtime"/sway-ipc.*.sock; do
    if [[ -S "$candidate" ]]; then ipc="$candidate"; break; fi
  done
  [[ -z "$ipc" ]] || break
  sleep 0.1
done
test -n "$ipc"
for title in TIMEW-TILE TIMEW-FLOAT; do
  "$STAGE_DIRECTOR_TEST_FOOT" --app-id=timew-capture-fixture --title="$title" --window-size-pixels=650x450 /usr/bin/sleep 30 >"$runtime/$title.log" 2>&1 &
  pids+=("$!")
done
if [[ "${TIMEW_CI_MULTI_CAPTURE:-0}" == 1 ]]; then
  # Wait for both real windows and the initial transaction to settle
  # before issuing ordinary resize/center to the exact disposable float.
  ready=0
  for _ in {1..100}; do
    if "$STAGE_DIRECTOR_TEST_SWAYMSG" -s "$ipc" -r -t get_tree >"$runtime/timew-current.json" &&
        python3 -B tests/timew/ci_assert_desktop_current.py "$runtime/timew-current.json" >/dev/null 2>&1; then
      ready=1
      break
    fi
    sleep 0.1
  done
  [[ "$ready" == 1 ]] || { echo "TIMEW_CI_MULTI_FAIL initial coverage" >&2; exit 1; }
  "$STAGE_DIRECTOR_TEST_SWAYMSG" -s "$ipc" -r \
    '[app_id="timew-capture-fixture" title="TIMEW-FLOAT"] resize set width 651px height 451px' >"$runtime/timew-resize.json"
  "$STAGE_DIRECTOR_TEST_SWAYMSG" -s "$ipc" -r \
    '[app_id="timew-capture-fixture" title="TIMEW-FLOAT"] move position center' >"$runtime/timew-center.json"
  python3 - "$runtime/timew-resize.json" "$runtime/timew-center.json" <<'PY'
import json,sys
for path in sys.argv[1:]:
    with open(path) as f:
        results=json.load(f)
    if len(results)!=1 or results[0].get("success") is not True:
        raise SystemExit("TIMEW_CI_MULTI_FAIL command rejected")
PY
  mode=(multi)
else
  mode=()
fi
for _ in {1..100}; do
  if "$STAGE_DIRECTOR_TEST_SWAYMSG" -s "$ipc" -r -t get_tree >"$runtime/timew-current.json" &&
      python3 -B tests/timew/ci_assert_desktop_current.py "$runtime/timew-current.json" "${mode[@]}" 2>"$runtime/timew-current.err"; then
    exit 0
  fi
  sleep 0.1
done
cat "$runtime/timew-current.err" >&2
cat "$runtime/timew-current.json" >&2
exit 1
