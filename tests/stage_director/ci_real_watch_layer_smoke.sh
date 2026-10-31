#!/usr/bin/env bash
# Actual mapped Weather layer + WATCH on the SAME wl_client, disposable CI.
set -euo pipefail
runtime="${1:?isolated XDG_RUNTIME_DIR}"
bin="${STAGE_DIRECTOR_TEST_WATCH_LAYER_CLIENT:-}"
spoof_pid=""
foreign_pid=""
weather_pid=""

cleanup() {
  local p
  for p in "$spoof_pid" "$foreign_pid" "$weather_pid"; do
    if [[ -n "$p" ]] && kill -0 "$p" 2>/dev/null; then
      kill -TERM "$p" 2>/dev/null || true
    fi
  done
  for p in "$spoof_pid" "$foreign_pid" "$weather_pid"; do
    if [[ -n "$p" ]]; then wait "$p" 2>/dev/null || true; fi
  done
}
trap cleanup EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ||
      "${XDG_RUNTIME_DIR:-}" != "$runtime" ||
      "${STAGE_DIRECTOR_CI_REAL_WATCH_LAYER:-}" != 1 ||
      ! -x "$bin" || "$bin" != /nix/store/* ||
      "${WAYLAND_DISPLAY:-}" != wayland-* ]]; then
  echo "BLOCKED_NONISOLATED_REAL_WATCH_LAYER" >&2
  exit 2
fi

"$bin" spoof >"$runtime/watch-spoof.log" 2>&1 &
spoof_pid=$!
"$bin" other >"$runtime/watch-other.log" 2>&1 &
foreign_pid=$!

ready=0
for _ in {1..100}; do
  if grep -Fq 'SD_CI_REAL_WATCH_LAYER_MAPPED role=spoof mapped=1' \
       "$runtime/watch-spoof.log" &&
     grep -Fq 'SD_CI_REAL_WATCH_LAYER_MAPPED role=other mapped=1' \
       "$runtime/watch-other.log"; then
    ready=1; break
  fi
  if ! kill -0 "$spoof_pid" 2>/dev/null ||
     ! kill -0 "$foreign_pid" 2>/dev/null; then break; fi
  sleep 0.05
done
if [[ "$ready" != 1 ]]; then
  echo "FAIL: cannot map spoof and other layer-shell peers" >&2
  cat "$runtime/watch-spoof.log" "$runtime/watch-other.log" >&2 || true
  tail -n 80 "$runtime/sway.log" >&2 || true
  exit 1
fi

"$bin" weather-watch >"$runtime/watch-weather.log" 2>&1 &
weather_pid=$!
test_result=0
for _ in {1..120}; do
  if grep -Fq 'SD_CI_REAL_WATCH_END_TO_END selected=1' \
       "$runtime/watch-weather.log"; then test_result=1; break; fi
  if ! kill -0 "$weather_pid" 2>/dev/null; then break; fi
  sleep 0.05
done
if [[ "$test_result" != 1 ]]; then
  echo "FAIL: real mapped Weather client did not complete WATCH/CLOSE" >&2
  cat "$runtime/watch-weather.log" >&2 || true
  tail -n 130 "$runtime/sway.log" >&2 || true
  exit 1
fi
if ! grep -Fq 'SD_CI_REAL_WATCH_SCENE selection=1 actual_mapped_layer=1 hit_test=1 authority=NONE' \
    "$runtime/sway.log"; then
  echo "FAIL: no actual compositor scene-hit selection evidence" >&2
  grep -E 'SD_CI_REAL_WATCH_SCENE|NEWS_HOST_PEER|layer reason=' \
    "$runtime/sway.log" >&2 || true
  exit 1
fi

# Preserve concrete logs and process exit status on client failures.
# This distinguishes real protocol/registry failure from shell cleanup.
if ! wait "$weather_pid"; then
  echo "FAIL: selected mapped Weather WATCH client exited unsuccessfully" >&2
  cat "$runtime/watch-weather.log" >&2 || true
  grep -E 'SD_CI_REAL_WATCH_SCENE|NEWS_HOST_PEER|post.apollo' "$runtime/sway.log" >&2 || true
  exit 1
fi
weather_pid=""
if ! wait "$spoof_pid"; then
  echo "FAIL: same-namespace unpaired layer client exited unsuccessfully" >&2
  cat "$runtime/watch-spoof.log" >&2 || true
  grep -E 'SD_CI_REAL_WATCH_SCENE|NEWS_HOST_PEER|post.apollo' "$runtime/sway.log" >&2 || true
  exit 1
fi
spoof_pid=""
if ! wait "$foreign_pid"; then
  echo "FAIL: other-namespace unpaired layer client exited unsuccessfully" >&2
  cat "$runtime/watch-other.log" >&2 || true
  grep -E 'SD_CI_REAL_WATCH_SCENE|NEWS_HOST_PEER|post.apollo' "$runtime/sway.log" >&2 || true
  exit 1
fi
foreign_pid=""

grep -F 'SD_CI_REAL_WATCH_END_TO_END' "$runtime/watch-weather.log"
grep -F 'SD_CI_REAL_WATCH_UNPAIRED role=spoof discovered=0 same_namespace_spoof=1 PASS' "$runtime/watch-spoof.log"
grep -F 'SD_CI_REAL_WATCH_UNPAIRED role=other discovered=0 same_namespace_spoof=0 PASS' "$runtime/watch-other.log"
grep -F 'SD_CI_REAL_WATCH_SCENE selection=1' "$runtime/sway.log" | tail -n 1
echo 'SD_CI_REAL_WATCH_FULL_COMPOSITOR actual_mapped_layer=1 exact_native_client=1 duplicate_namespace_rejected=1 known_watch=1 close_revoked=1 browser=OFF input_auth=UNTESTED result=PASS'
