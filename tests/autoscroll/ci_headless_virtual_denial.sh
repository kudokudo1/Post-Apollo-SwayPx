#!/usr/bin/env bash
# Actual SwayPX headless virtual-pointer NEGATIVE ingress test, CI-only.
# This proves normal BTN_MIDDLE press/release and no unsolicited axis while
# PA_AUTOSCROLL_LIVE_ACTIVATION=0. Virtual source is NOT physical authority.
set -euo pipefail

sway="${1:?Nix-built SwayPX binary required}"
swaymsg="${2:?Nix-built swaymsg required}"
observer="${3:?Nix-built observer required}"
sender="${4:?Nix-built sender required}"

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "${RUNNER_TEMP}" != /* ||
      ! -x "$sway" || ! -x "$swaymsg" ||
      ! -x "$observer" || ! -x "$sender" ]]; then
  echo "PA_AUTOSCROLL_BLOCKED_NON_CI_OR_MISSING_BINARIES" >&2
  exit 2
fi
runtime="$(mktemp -d "$RUNNER_TEMP/pa-autoscroll-v5c-negative.XXXXXXXX")"
chmod 700 "$runtime"
config="$runtime/config"
log="$runtime/sway.log"
client_log="$runtime/observer.log"
sway_pid=""
client_pid=""
success=0
device=/dev/udmabuf
acl_changed=0
acl_snapshot=""
cleanup() {
  local original_rc=$?
  trap - EXIT
  if [[ -n "$client_pid" ]]; then
    kill -TERM "$client_pid" 2>/dev/null || true
    wait "$client_pid" 2>/dev/null || true
  fi
  if [[ -n "$sway_pid" ]]; then
    kill -TERM "$sway_pid" 2>/dev/null || true
    for _ in {1..40}; do
      kill -0 "$sway_pid" 2>/dev/null || break
      sleep 0.1
    done
    kill -KILL "$sway_pid" 2>/dev/null || true
    wait "$sway_pid" 2>/dev/null || true
  fi
  if [[ "$success" -ne 1 ]]; then
    echo "=== V5C HEADLESS OBSERVER LOG ===" >&2
    tail -80 "$client_log" 2>/dev/null || true
    echo "=== V5C COMPOSITOR LOG ===" >&2
    tail -120 "$log" 2>/dev/null || true
  fi
  # If an isolated CI-only ACL was created, restore its exact snapshot
  # BEFORE removing the private runtime. Restoration failure is not ignorable.
  if [[ "$acl_changed" == 1 ]]; then
    if ! sudo -n setfacl --restore="$acl_snapshot"; then
      echo "PA_AUTOSCROLL_CI_UDMABUF_ACL_RESTORE_FAILED" >&2
      original_rc=3
    fi
  fi
  rm -rf -- "$runtime"
  exit "$original_rc"
}
trap cleanup EXIT

# The pinned SceneFX software path requires /dev/udmabuf. Reuse the
# established Stage Director guarded ephemeral-runner ACL pattern without
# touching any operator workstation. Never chmod/chown the character device.
if [[ ! -c "$device" || "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "PA_AUTOSCROLL_CI_UDMABUF_UNEXPECTED_DEVICE" >&2
  exit 2
fi
if ! python3 -c 'import os; fd=os.open("/dev/udmabuf",os.O_RDWR|os.O_CLOEXEC); os.close(fd)' 2>/dev/null; then
  if ! command -v getfacl >/dev/null || ! command -v setfacl >/dev/null; then
    echo "PA_AUTOSCROLL_CI_MISSING_ACL_TOOLS" >&2
    exit 2
  fi
  if getfacl -cp "$device" | grep -Eq "^user:$(id -u):"; then
    echo "PA_AUTOSCROLL_CI_EXISTING_DEVICE_ACL_REFUSED" >&2
    exit 2
  fi
  acl_snapshot="$runtime/udmabuf-original.acl"
  getfacl -p "$device" > "$acl_snapshot"
  acl_changed=1
  if ! sudo -n setfacl -m "u:$(id -u):rw" "$device"; then
    echo "PA_AUTOSCROLL_CI_UDMABUF_ACL_DENIED" >&2
    exit 2
  fi
  if ! python3 -c 'import os; fd=os.open("/dev/udmabuf",os.O_RDWR|os.O_CLOEXEC); os.close(fd)' 2>/dev/null; then
    echo "PA_AUTOSCROLL_CI_UDMABUF_OPEN_DENIED" >&2
    exit 2
  fi
  echo "PA_AUTOSCROLL_CI_TEMPORARY_UDMABUF_ACL_GRANTED"
fi

cat > "$config" <<'EOF'
font pango:monospace 10
output * bg #000000 solid_color
output * resolution 1920x1080
for_window [app_id="pa-ci-autoscroll-observer"] floating enable
EOF

mesa_dir="${STAGE_DIRECTOR_MESA_DIR:-}"
if [[ "$mesa_dir" != /nix/store/* ||
      ! -f "$mesa_dir/share/glvnd/egl_vendor.d/50_mesa.json" ||
      ! -d "$mesa_dir/lib/dri" ||
      ! -f "$mesa_dir/lib/libEGL_mesa.so.0" ]]; then
  echo "PA_AUTOSCROLL_BLOCKED_UNPINNED_RENDERER" >&2
  exit 2
fi
export XDG_RUNTIME_DIR="$runtime"
export XDG_SESSION_TYPE=wayland
export WLR_BACKENDS=headless
export WLR_HEADLESS_OUTPUTS=1
export WLR_LIBINPUT_NO_DEVICES=1
export WLR_RENDERER_FORCE_SOFTWARE=1
export WLR_RENDERER_ALLOW_SOFTWARE=1
export LIBGL_ALWAYS_SOFTWARE=1
export EGL_PLATFORM=surfaceless
export __EGL_VENDOR_LIBRARY_FILENAMES="$mesa_dir/share/glvnd/egl_vendor.d/50_mesa.json"
export LIBGL_DRIVERS_PATH="$mesa_dir/lib/dri"
export LD_LIBRARY_PATH="$mesa_dir/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
unset DISPLAY WAYLAND_DISPLAY WAYLAND_SOCKET SWAYSOCK WLR_RENDERER WLR_RENDER_DRM_DEVICE

"$sway" -d -c "$config" >"$log" 2>&1 &
sway_pid=$!
wayland=""
ipc=""
for _ in {1..150}; do
  kill -0 "$sway_pid" 2>/dev/null || {
    echo "PA_AUTOSCROLL_NO_COMPOSITOR_START" >&2
    exit 1
  }
  for candidate in "$runtime"/wayland-*; do
    if [[ -S "$candidate" ]]; then wayland="${candidate##*/}"; fi
  done
  for candidate in "$runtime"/sway-ipc.*.sock; do
    if [[ -S "$candidate" ]]; then ipc="$candidate"; fi
  done
  [[ -n "$wayland" && -n "$ipc" ]] && break
  sleep 0.1
done
if [[ -z "$wayland" || -z "$ipc" ]]; then
  echo "PA_AUTOSCROLL_MISSING_PRIVATE_WAYLAND_IPC" >&2
  exit 1
fi
export WAYLAND_DISPLAY="$wayland"
export SWAYSOCK="$ipc"
export PA_AUTOSCROLL_CI_VIRTUAL_NEGATIVE=1

"$observer" run >"$client_log" 2>&1 &
client_pid=$!
for _ in {1..100}; do
  kill -0 "$client_pid" 2>/dev/null || {
    echo "PA_AUTOSCROLL_OBSERVER_EXITED_BEFORE_MAP" >&2
    exit 1
  }
  if grep -q 'PA_CI_AUTOSCROLL_NATIVE_MAPPED' "$client_log"; then break; fi
  sleep 0.1
done
if ! grep -q 'PA_CI_AUTOSCROLL_NATIVE_MAPPED' "$client_log"; then
  echo "PA_AUTOSCROLL_OBSERVER_NOT_MAPPED" >&2
  exit 1
fi

"$swaymsg" -s "$ipc" \
  '[app_id="pa-ci-autoscroll-observer"] move position 420 280' >/dev/null
sleep 0.3

# Use the actual Sway compositor tree, never assume pointer focus by app_id.
coords="$("$swaymsg" -s "$ipc" -t get_tree -r | python3 -c '
import json,sys
root=json.load(sys.stdin)
def walk(node):
    yield node
    for key in ("nodes","floating_nodes"):
        for child in node.get(key,[]): yield from walk(child)
matches=[n for n in walk(root) if n.get("app_id")=="pa-ci-autoscroll-observer" and n.get("visible",False)]
if len(matches)!=1: raise SystemExit("PA_CI_TARGET_NOT_UNIQUE")
rect=matches[0].get("rect") or {}
x,y=rect.get("x",-1)+60,rect.get("y",-1)+65
if not (0 <= x < 1920 and 0 <= y < 1080):
    raise SystemExit("PA_CI_TARGET_COORD_INVALID")
print(x,y)
')"
read -r x y <<< "$coords"
echo "PA_AUTOSCROLL_CI_REAL_TARGET coordinate_from_compositor=1 x=$x y=$y"

# The only injected pointer is zwlr_virtual_pointer_v1, never libinput.
timeout 12s "$sender" middle "$x" "$y"
sleep 1

if ! grep -q 'PA_CI_AUTOSCROLL_RECEIPT kind=ENTER ' "$client_log" ||
   ! grep -q 'PA_CI_AUTOSCROLL_RECEIPT kind=BUTTON code=274 state=PRESS ' "$client_log" ||
   ! grep -q 'PA_CI_AUTOSCROLL_RECEIPT kind=BUTTON code=274 state=RELEASE ' "$client_log"; then
  echo "PA_AUTOSCROLL_VIRTUAL_BUTTON_NOT_DELIVERED_TO_NORMAL_CLIENT" >&2
  exit 1
fi
if grep -q 'PA_CI_AUTOSCROLL_RECEIPT kind=AXIS ' "$client_log"; then
  echo "PA_AUTOSCROLL_SPURIOUS_AXIS_WHILE_HARD_OFF" >&2
  exit 1
fi
echo "PA_AUTOSCROLL_VIRTUAL_MIDDLE_PASSTHROUGH_PASS normal_button=1 axis=0 physical_authority=NONE"
success=1
