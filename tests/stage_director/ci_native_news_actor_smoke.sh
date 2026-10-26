#!/usr/bin/env bash
# Map a deliberately oversized disposable *native Wayland* actor inside the
# CI-only SwayPX headless compositor. This is NOT browser playback, clipping,
# popup hiding or trusted process authentication.
set -euo pipefail
runtime="${1:?private XDG_RUNTIME_DIR}"
wayland_socket="${2:?Wayland display socket basename}"
foot_binary="${STAGE_DIRECTOR_TEST_FOOT:-}"
swaymsg_binary="${STAGE_DIRECTOR_TEST_SWAYMSG:-}"
actor=""
actor_log="$runtime/news-actor.log"
tree_json="$runtime/news-tree.json"

cleanup_actor() {
  if [[ -n "$actor" ]]; then
    if kill -0 "$actor" 2>/dev/null; then
      kill -TERM "$actor" 2>/dev/null || true
      for _ in {1..30}; do
        if ! kill -0 "$actor" 2>/dev/null; then break; fi
        sleep 0.1
      done
      if kill -0 "$actor" 2>/dev/null; then
        kill -KILL "$actor" 2>/dev/null || true
      fi
    fi
    wait "$actor" 2>/dev/null || true
  fi
}
trap cleanup_actor EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ||
      ! -d "$runtime" || ! -x "$foot_binary" ||
      ! -x "$swaymsg_binary" || "$wayland_socket" != wayland-* ]]; then
  echo "BLOCKED_UNSAFE_ACTOR_ENV: require disposable CI session and fixed binaries" >&2
  exit 2
fi

ipc_socket=""
for _ in {1..60}; do
  for candidate in "$runtime"/sway-ipc.*.sock; do
    if [[ -S "$candidate" ]]; then
      ipc_socket="$candidate"
      break
    fi
  done
  if [[ -n "$ipc_socket" ]]; then break; fi
  sleep 0.1
done
if [[ -z "$ipc_socket" ]]; then
  echo "BLOCKED_NO_SWAY_IPC: compositor Wayland socket exists but IPC was not registered" >&2
  exit 2
fi

# Keep the test actor's environment private from the GitHub runner account.
mkdir -m 700 -p "$runtime/home" "$runtime/config-home"
export HOME="$runtime/home"
export XDG_CONFIG_HOME="$runtime/config-home"
export XDG_RUNTIME_DIR="$runtime"
export WAYLAND_DISPLAY="$wayland_socket"
export SWAYSOCK="$ipc_socket"
unset DISPLAY

# App-id imitation intentionally tests classification as CANDIDATE ONLY.
# It is a native xdg_toplevel terminal running a fixed sleep test command,
# NOT a browser, not a URL, and not a trusted WEATHER WATCH child.
"$foot_binary" \
  --app-id=post-apollo-weather-news \
  --title=SD1-CI-DISPOSABLE-NOT-BROWSER \
  --window-size-pixels=1400x850 \
  /usr/bin/sleep 30 >"$actor_log" 2>&1 &
actor=$!

mapped=0
for _ in {1..90}; do
  if ! kill -0 "$actor" 2>/dev/null; then
    echo "Wayland test actor exited before mapping:" >&2
    tail -n 70 "$actor_log" >&2
    exit 1
  fi
  if "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree >"$tree_json" 2>/dev/null &&
      python3 tests/stage_director/assert_native_news_actor.py "$tree_json" >/dev/null 2>&1; then
    mapped=1
    break
  fi
  sleep 0.1
done
if [[ "$mapped" -ne 1 ]]; then
  echo "Native Wayland NEWS test actor did not map with an oversized rect" >&2
  tail -n 70 "$actor_log" >&2
  python3 tests/stage_director/assert_native_news_actor.py "$tree_json" >&2 || true
  exit 1
fi

python3 tests/stage_director/assert_native_news_actor.py "$tree_json"
echo "Stage Director real mapped actor smoke: PASS"
echo "No scene clipping, popup filtering or seat/input interception activated."
