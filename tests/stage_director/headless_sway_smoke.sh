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
# Isolated virtual monitor can host a candidate actor wider than NEWS bay.
output * resolution 1920x1080
for_window [app_id="post-apollo-weather-news"] floating enable
EOF

# Two independent compositor boots use different virtual output sizes.
# Never resize a running headless output: pinned Mesa llvmpipe crashes on
# the original mode-change experiment in this disposable CI environment.
cold_size="${STAGE_DIRECTOR_CI_COLD_SIZE:-1920x1080}"
if [[ "$cold_size" != 1920x1080 && "$cold_size" != 1888x1056 ]]; then
  echo "BLOCKED_UNSUPPORTED_CI_COLD_SIZE: $cold_size" >&2
  exit 2
fi
if [[ "$cold_size" == 1888x1056 ]]; then
  if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
        -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ]]; then
    echo "BLOCKED_COLD_OUTPUT_OUTSIDE_CI" >&2
    exit 2
  fi
  sed -i 's/output \* resolution 1920x1080/output * resolution 1888x1056/' "$config"
fi
echo "SD_CI_COLD_BOOT virtual_output=$cold_size"

export XDG_RUNTIME_DIR="$runtime"
export XDG_SESSION_TYPE=wayland
export WLR_BACKENDS=headless
export WLR_HEADLESS_OUTPUTS=1
export WLR_LIBINPUT_NO_DEVICES=1
# This exact fallback exists in pinned SceneFX 26200c5: its
# open_preferred_drm_fd() handles WLR_RENDERER_FORCE_SOFTWARE by bypassing
# DRM probing. WLR_RENDERER=pixman alone DOES NOT affect fx_renderer_create().
# Keep this limited to the temporary CI test, never production SwayPX.
export WLR_RENDERER_FORCE_SOFTWARE=1
export WLR_RENDERER_ALLOW_SOFTWARE=1
export LIBGL_ALWAYS_SOFTWARE=1
export EGL_PLATFORM=surfaceless
# Nix's unwrapped compositor does not inherit the graphical vendor/driver
# discovery paths used inside NixOS. Source both from its *pinned* nixpkgs,
# never the GitHub runner's unrelated host /usr/lib graphics stack.
mesa_dir="${STAGE_DIRECTOR_MESA_DIR:-}"
if [[ -z "$mesa_dir" || "$mesa_dir" != /nix/store/* ||
      ! -r "$mesa_dir/share/glvnd/egl_vendor.d/50_mesa.json" ||
      ! -d "$mesa_dir/lib/dri" ||
      ! -r "$mesa_dir/lib/libEGL_mesa.so.0" ]]; then
  echo "BLOCKED_NIX_MESA_VENDOR: pinned EGL vendor JSON / Mesa DRI driver paths unavailable" >&2
  exit 2
fi
export __EGL_VENDOR_LIBRARY_FILENAMES="$mesa_dir/share/glvnd/egl_vendor.d/50_mesa.json"
export LIBGL_DRIVERS_PATH="$mesa_dir/lib/dri"
export LD_LIBRARY_PATH="$mesa_dir/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
unset DISPLAY WAYLAND_DISPLAY WAYLAND_SOCKET SWAYSOCK WLR_RENDERER WLR_RENDER_DRM_DEVICE

"$sway_binary" --version
echo "Stage Director CI: testing pinned SceneFX software EGL path without DRM"
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
echo "Headless startup alone does not prove NEWS clip/input/popup rendering."

if [[ -n "${STAGE_DIRECTOR_TEST_FOOT:-}" ]]; then
  bash tests/stage_director/ci_native_news_actor_smoke.sh \
    "$runtime" "${socket##*/}"
fi

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
