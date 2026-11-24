#!/usr/bin/env bash
# Test-only /dev/udmabuf ACL for a disposable GitHub hosted compositor.
# Never execute on user's workstation. Every ACL change is restored.
set -euo pipefail
device=/dev/udmabuf
acl_snapshot=""
acl_changed=0
uid="$(id -u)"

cleanup() {
  local rc=$?
  trap - EXIT
  if [[ "$acl_changed" == 1 ]]; then
    if sudo -n setfacl --restore="$acl_snapshot"; then
      echo "PA_FC1_CI_UDMABUF original_acl=RESTORED"
    else
      echo "::error title=PA_FC1_ACL_RESTORE_FAILED::Runner device ACL not restored" >&2
      rc=3
    fi
  fi
  if [[ -n "$acl_snapshot" ]]; then rm -f -- "$acl_snapshot"; fi
  exit "$rc"
}
trap cleanup EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      "${PA_AUTOSCROLL_CI_NATIVE_TRACE:-}" != 1 ||
      "${RUNNER_TEMP:-}" != /home/runner/work/_temp ||
      "$uid" == 0 || "${PA_FC1_SWAY:-}" != /nix/store/* ||
      "${PA_FC1_SWAYMSG:-}" != /nix/store/* ||
      "${PA_FC1_FOOT:-}" != /nix/store/* ||
      "${PA_FC1_MESA:-}" != /nix/store/* ]]; then
  echo "BLOCKED_FC1_UDMABUF_NOT_DISPOSABLE_CI" >&2
  exit 2
fi
if [[ ! -c "$device" || ! -x "$PA_FC1_SWAY" ||
      "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "BLOCKED_FC1_UNSAFE_UDMABUF_OR_BINARY" >&2
  exit 2
fi
can_open() {
  python3 -c 'import os; fd=os.open("/dev/udmabuf",os.O_RDWR|os.O_CLOEXEC); os.close(fd)' \
    >/dev/null 2>&1
}
if ! can_open; then
  if ! command -v getfacl >/dev/null || ! command -v setfacl >/dev/null; then
    echo "BLOCKED_FC1_ACL_TOOLS_NOT_PRESENT" >&2
    exit 2
  fi
  if getfacl -cp "$device" | grep -Eq "^user:$uid:"; then
    echo "BLOCKED_FC1_PREEXISTING_RUNNER_ACL" >&2
    exit 2
  fi
  acl_snapshot="$(mktemp "$RUNNER_TEMP/pa-fc1-acl.XXXXXXXX")"
  getfacl -p "$device" > "$acl_snapshot"
  acl_changed=1
  sudo -n setfacl -m "u:$uid:rw" "$device"
  if ! can_open; then
    echo "BLOCKED_FC1_UDMABUF_STILL_DENIED" >&2
    exit 2
  fi
  echo "PA_FC1_UDMABUF temporary_CI_only_access=GRANTED"
else
  echo "PA_FC1_UDMABUF original_access=SUFFICIENT"
fi

python3 -B tests/autoscroll/ci_v5f_c1_native_trace.py
