#!/usr/bin/env bash
# Stage Director isolated SwayPX runtime smoke. GitHub-hosted runner ONLY.
# Exercises compositor startup / Wayland socket / controlled teardown.
# Does NOT launch Quickshell, browser, NEWS or claim visual containment.
set -euo pipefail

sway_binary="${1:?pass the Nix-built sway binary path}"
if [[ ! -x "$sway_binary" ]]; then
  echo "SwayPX binary is not executable: $sway_binary" >&2
  exit 1
fi

runtime="$(mktemp -d "${RUNNER_TEMP:-/tmp}/stage-director-headless.XXXXXXXX")"
chmod 700 "$runtime"
log="$runtime/sway.log"
config="$runtime/config"
child=""
success=0

cleanup() {
  if [[ -n "$child" ]] && kill -0 "$child" 2>/dev/null; then
    kill -TERM "$child" 2>/dev/null || true
    for _ in {1..40}; do
      if ! kill -0 "$child" 2>/dev/null; then
        break
      fi
      sleep 0.1
    done
    if kill -0 "$child" 2>/dev/null; then
      kill -KILL "$child" 2>/dev/null || true
    fi
  fi
  if [[ -n "$child" ]]; then
    wait "$child" 2>/dev/null || true
  fi
  if [[ "$success" -ne 1 ]]; then
    echo "Stage Director headless compositor logs:" >&2
    if [[ -f "$log" ]]; then
      tail -n 120 "$log" >&2
    fi
  fi
  rm -rf -- "$runtime"
}
trap cleanup EXIT

cat > "$config" <<'EOF'
# Isolated CI config. Never read or modify a user's Sway configuration.
font pango:monospace 10
output * bg #000000 solid_color
EOF

export XDG_RUNTIME_DIR="$runtime"
export XDG_SESSION_TYPE=wayland
export WLR_BACKENDS=headless
export WLR_HEADLESS_OUTPUTS=1
export WLR_LIBINPUT_NO_DEVICES=1
export WLR_RENDERER=pixman
export WLR_RENDERER_ALLOW_SOFTWARE=1
unset DISPLAY WAYLAND_DISPLAY WAYLAND_SOCKET SWAYSOCK

"$sway_binary" --version
render_available=0
for render_node in /dev/dri/renderD*; do
  if [[ -c "$render_node" && -r "$render_node" && -w "$render_node" ]]; then
    render_available=1
    break
  fi
done
if [[ "$render_available" -ne 1 ]]; then
  echo "BLOCKED_NO_DRM_RENDER_NODE: SceneFX needs a usable DRM render device for GLES2. This machine cannot perform a live compositor rendering test." >&2
  exit 2
fi
"$sway_binary" -d -c "$config" > "$log" 2>&1 &
child=$!

socket=""
for _ in {1..150}; do
  if ! kill -0 "$child" 2>/dev/null; then
    echo "SwayPX exited before opening a Wayland display" >&2
    exit 1
  fi
  for candidate in "$runtime"/wayland-*; do
    if [[ -S "$candidate" ]]; then
      socket="$candidate"
      break
    fi
  done
  if [[ -n "$socket" ]]; then
    break
  fi
  sleep 0.1
done

if [[ -z "$socket" ]]; then
  echo "No Wayland display socket appeared within the bounded test window" >&2
  exit 1
fi

# Catch immediate crash after socket creation without running any application.
sleep 0.5
if ! kill -0 "$child" 2>/dev/null; then
  echo "SwayPX exited immediately after creating the Wayland socket" >&2
  exit 1
fi

echo "Stage Director headless compositor initialized a Wayland socket: ${socket##*/}"
echo "This verifies startup only; no NEWS clip/input/popup rendering tested."

kill -TERM "$child"
for _ in {1..40}; do
  if ! kill -0 "$child" 2>/dev/null; then
    break
  fi
  sleep 0.1
done
if kill -0 "$child" 2>/dev/null; then
  echo "SwayPX did not terminate within the bounded shutdown interval" >&2
  exit 1
fi
wait "$child" 2>/dev/null || true
child=""
success=1
echo "Stage Director headless compositor startup and shutdown smoke: PASS"
