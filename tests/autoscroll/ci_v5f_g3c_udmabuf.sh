#!/usr/bin/env bash
# G3C runs ONLY on disposable GitHub hosted CI; restore original device ACL.
set -euo pipefail
device=/dev/udmabuf
snapshot=""
changed=0
uid="$(id -u)"
cleanup() {
  local rc=$?
  trap - EXIT
  if [[ "$changed" == 1 ]]; then
    if sudo -n setfacl --restore="$snapshot"; then
      echo "PA_G3C_CI_UDMABUF original_acl=RESTORED"
    else
      echo "::error title=PA_G3C_ACL_RESTORE_FAILED::CI runner device ACL not restored" >&2
      rc=3
    fi
  fi
  if [[ -n "$snapshot" ]]; then rm -f -- "$snapshot"; fi
  exit "$rc"
}
trap cleanup EXIT
if [[ "${GITHUB_ACTIONS:-}" != true ||
      "${CI:-}" != true ||
      "${PA_AUTOSCROLL_CI_NATIVE_TRACE:-}" != 1 ||
      "${PA_G3C_NATIVE_FOUR_OWNER:-}" != 1 ||
      "${RUNNER_TEMP:-}" != /home/runner/work/_temp ||
      "$uid" == 0 ||
      "${PA_FC1_SWAY:-}" != /nix/store/* ||
      "${PA_FC1_SWAYMSG:-}" != /nix/store/* ||
      "${PA_FC1_FOOT:-}" != /nix/store/* ||
      "${PA_G3C_CLIENT:-}" != "$RUNNER_TEMP"/* ||
      "${PA_FC1_MESA:-}" != /nix/store/* ]]; then
  echo "BLOCKED_G3C_UDMABUF_NOT_DISPOSABLE_CI" >&2
  exit 2
fi
if [[ ! -c "$device" || ! -x "$PA_FC1_SWAY" ||
      "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "BLOCKED_G3C_UNSAFE_UDMABUF_OR_BINARY" >&2
  exit 2
fi
can_open() {
  python3 -c 'import os; fd=os.open("/dev/udmabuf",os.O_RDWR|os.O_CLOEXEC); os.close(fd)' >/dev/null 2>&1
}
if ! can_open; then
  command -v getfacl >/dev/null || exit 2
  command -v setfacl >/dev/null || exit 2
  if getfacl -cp "$device" | grep -Eq "^user:$uid:"; then
    echo "BLOCKED_G3C_PREEXISTING_RUNNER_ACL" >&2
    exit 2
  fi
  snapshot="$(mktemp "$RUNNER_TEMP/pa-g3c-acl.XXXXXXXX")"
  getfacl -p "$device" > "$snapshot"
  changed=1
  sudo -n setfacl -m "u:$uid:rw" "$device"
  can_open || { echo "BLOCKED_G3C_UDMABUF_STILL_DENIED" >&2; exit 2; }
  echo "PA_G3C_CI_UDMABUF temporary_runner_only=GRANTED"
else
  echo "PA_G3C_CI_UDMABUF original_access=SUFFICIENT"
fi
test -x "$PA_G3C_CLIENT"
python3 -B tests/autoscroll/ci_v5f_g3c_native_trace.py
