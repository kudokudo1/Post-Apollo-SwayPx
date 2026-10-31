#!/usr/bin/env bash
# Native layer-shell mapping and namespace collision, disposable CI only.
# This never launches a NEWS browser or authenticates a WATCH request.
set -euo pipefail

runtime="${1:?private CI compositor runtime}"
actor_bin="${STAGE_DIRECTOR_TEST_LAYER_HOST:-}"
real_pid=""
spoof_pid=""
other_pid=""

cleanup() {
  local p
  for p in "$real_pid" "$spoof_pid" "$other_pid"; do
    if [[ -n "$p" ]] && kill -0 "$p" 2>/dev/null; then
      kill -TERM "$p" 2>/dev/null || true
    fi
  done
  for p in "$real_pid" "$spoof_pid" "$other_pid"; do
    if [[ -n "$p" ]]; then wait "$p" 2>/dev/null || true; fi
  done
}
trap cleanup EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ||
      ! -d "$runtime" || ! -x "$actor_bin" ||
      "${XDG_RUNTIME_DIR:-}" != "$runtime" ||
      "${WAYLAND_DISPLAY:-}" != wayland-* ]]; then
  echo "BLOCKED_NONISOLATED_NATIVE_WEATHER_HOST" >&2
  exit 2
fi

# Make three distinct REAL native wl_clients, including two deliberately
# indistinguishable namespace strings. Only the CI parent owns these PIDs.
"$actor_bin" weather >"$runtime/sd1-layer-weather.log" 2>&1 &
real_pid=$!
"$actor_bin" spoof >"$runtime/sd1-layer-spoof.log" 2>&1 &
spoof_pid=$!
"$actor_bin" other >"$runtime/sd1-layer-other.log" 2>&1 &
other_pid=$!

ready=0
for _ in {1..100}; do
  if grep -Fq "SD_CI_LAYER_HOST_READY role=weather pid=$real_pid " \
      "$runtime/sd1-layer-weather.log" &&
      grep -Fq "SD_CI_LAYER_HOST_READY role=spoof pid=$spoof_pid " \
      "$runtime/sd1-layer-spoof.log" &&
      grep -Fq "SD_CI_LAYER_HOST_READY role=other pid=$other_pid " \
      "$runtime/sd1-layer-other.log"; then
    ready=1
    break
  fi
  if ! kill -0 "$real_pid" 2>/dev/null ||
      ! kill -0 "$spoof_pid" 2>/dev/null ||
      ! kill -0 "$other_pid" 2>/dev/null; then break; fi
  sleep 0.1
done
if [[ "$ready" != 1 ]]; then
  echo "FAIL: one of three fixed native layer-shell clients did not map" >&2
  for role in weather spoof other; do
    echo "===== $role =====" >&2
    cat "$runtime/sd1-layer-$role.log" >&2 || true
  done
  tail -n 70 "$runtime/sway.log" >&2
  exit 1
fi

found=0
for _ in {1..100}; do
  if python3 - "$runtime/sway.log" \
      "$real_pid" "$spoof_pid" "$other_pid" >/dev/null 2>&1 <<'PY'
import pathlib, re, sys
log=pathlib.Path(sys.argv[1]).read_text(encoding="utf-8",errors="replace")
real,spoof,other=(int(s) for s in sys.argv[2:])
rows=re.findall(
    r"\[stage-director\]\[SD1\]\[NEWS_HOST_PEER\][^\n]*"
    r"native_peer=candidate pid=(\d+) uid=(\d+) start=(\d+)"
    r"[^\n]*operator_authenticated=no can_launch=no "
    r"can_bind_viewer=no can_clip=no can_accept_input=no",log)
pids={int(pid) for pid,uid,start in rows if int(uid)>0 and int(start)>0}
if real not in pids or spoof not in pids or other in pids:
    raise SystemExit("not all native host peers observed")
PY
  then
    found=1
    break
  fi
  sleep 0.1
done
if [[ "$found" != 1 ]]; then
  echo "FAIL: native wl_client credentials were not captured for both spoofable namespace peers" >&2
  grep -E "NEWS_HOST_PEER|layer reason=.*namespace=post-apollo" \
    "$runtime/sway.log" >&2 || true
  exit 1
fi

python3 - "$runtime/sway.log" "$real_pid" "$spoof_pid" "$other_pid" <<'PY'
import pathlib, re, sys
log=pathlib.Path(sys.argv[1]).read_text(encoding="utf-8",errors="replace")
real,spoof,other=(int(s) for s in sys.argv[2:])
rows=re.findall(
    r"\[stage-director\]\[SD1\]\[NEWS_HOST_PEER\][^\n]*"
    r"native_peer=candidate pid=(\d+) uid=(\d+) start=(\d+)"
    r"[^\n]*operator_authenticated=no can_launch=no "
    r"can_bind_viewer=no can_clip=no can_accept_input=no",log)
seen={int(pid) for pid,uid,start in rows if int(uid)>0 and int(start)>0}
if real == spoof or not {real,spoof}.issubset(seen) or other in seen:
    raise SystemExit(f"FAIL: missing native double-candidate or foreign matched: {seen}")
if f"namespace=post-apollo-different-layer" not in log:
    raise SystemExit("FAIL: wrong-namespace native layer was not observed")
if not re.search(r"\[stage-director\]\[SD1\]\[NEWS\].*hosts=2 actors=0", log):
    raise SystemExit("FAIL: two same-namespace hosts not simultaneously observed")
print("SD_CI_LAYER_HOST_CHECK native_lookalike_clients=2 "
      "foreign_namespace_excluded=1 compositor_peer_verified=1 "
      "watch_origin_auth=NONE authority=NONE result=PASS")
PY

for p in "$real_pid" "$spoof_pid" "$other_pid"; do
  kill -TERM "$p"
done
wait "$real_pid" 2>/dev/null || true
wait "$spoof_pid" 2>/dev/null || true
wait "$other_pid" 2>/dev/null || true
real_pid=""
spoof_pid=""
other_pid=""
removed=0
for _ in {1..100}; do
  if grep -Eq '\[stage-director\]\[SD1\]\[NEWS\].*hosts=0 actors=0' \
      "$runtime/sway.log"; then
    removed=1
    break
  fi
  sleep 0.1
done
if [[ "$removed" != 1 ]]; then
  echo "FAIL: native duplicate Weather host candidates were not removed" >&2
  grep -E "NEWS_HOST_PEER|\[SD1\]\[NEWS\]" "$runtime/sway.log" >&2 || true
  exit 1
fi
echo "SD_CI_LAYER_HOST_UNMAP both_namespace_candidates_cleared=1 unrelated_layer_closed=1 authority=NONE result=PASS"
