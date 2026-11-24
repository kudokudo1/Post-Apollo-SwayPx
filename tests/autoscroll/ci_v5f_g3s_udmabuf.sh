#!/usr/bin/env bash
# Disposable GitHub runner only; temporary /dev/udmabuf access restored.
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
      echo "PA_G3M_RUNNER_UDMABUF_ACL_RESTORED"
    else
      echo "::error title=PA_G3M_ACL_RESTORE_FAILED::runner ACL not restored" >&2
      rc=3
    fi
  fi
  if [[ -n "$snapshot" ]]; then rm -f -- "$snapshot"; fi
  exit "$rc"
}
trap cleanup EXIT
test "${RUNNER_ENVIRONMENT:-}" = github-hosted
test "${PA_G3O_NATIVE_EPOCH:-}" = 1
test "${PA_G3P_POST_ABA:-}" = 1
if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      "${PA_G2_NATIVE_BUTTON:-}" != 2 ||
      "${PA_AUTOSCROLL_CI_NATIVE_TRACE:-}" != 2 ||
      "${RUNNER_TEMP:-}" != /home/runner/work/_temp ||
      "$uid" == 0 || "${PA_G2_SWAY:-}" != /nix/store/* ||
      "${PA_G3J_ACTOR_BINARY:-}" != "$RUNNER_TEMP"/* ||
      "${PA_G2_MESA:-}" != /nix/store/* ||
      "${PA_G3J_NATIVE_LIBINPUT_DIAG:-}" != 1 ||
      "${PA_G3K_NATIVE_GUARD:-}" != 1 ||
      "${PA_G3M_NATIVE_OBSERVE:-}" != 1 ||
      "${PA_G3L_NATIVE_OBSERVE:-}" != 1 ||
      "${PA_G3M_KERNEL_ACTOR:-}" != 1 ||
      "${PA_G3K_KERNEL_ACTOR:-}" != 1 ||
      "${PA_G3E_SWAYMSG:-}" != /nix/store/* ||
      "${PA_G3E_FOOT:-}" != /nix/store/* ]]; then
  echo "BLOCKED_G3M_UDMABUF_NOT_DISPOSABLE_CI" >&2
  exit 2
fi
if [[ ! -c "$device" || ! -x "$PA_G2_SWAY" ||
      "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "BLOCKED_G3M_UNSAFE_UDMABUF_OR_BINARY" >&2
  exit 2
fi
can_open() {
  python3 -c 'import os; fd=os.open("/dev/udmabuf",os.O_RDWR|os.O_CLOEXEC); os.close(fd)' >/dev/null 2>&1
}
if ! can_open; then
  command -v getfacl >/dev/null || exit 2
  command -v setfacl >/dev/null || exit 2
  if getfacl -cp "$device" | grep -Eq "^user:$uid:"; then
    echo "BLOCKED_G3M_PREEXISTING_RUNNER_ACL" >&2
    exit 2
  fi
  snapshot="$(mktemp "$RUNNER_TEMP/pa-g3m-acl.XXXXXXXX")"
  getfacl -p "$device" > "$snapshot"
  changed=1
  sudo -n setfacl -m "u:$uid:rw" "$device"
  can_open || { echo "BLOCKED_G3M_UDMABUF_STILL_DENIED" >&2; exit 2; }
  echo "PA_G3M_RUNNER_UDMABUF_CI_TEMPORARY_ACCESS"
else
  echo "PA_G3M_RUNNER_UDMABUF_ALREADY_ACCESSIBLE"
fi
test "${PA_G3Q_GROUP:-}" = 1
test "${PA_G3R_SEAT:-}" = 1
test "${PA_G3S_LOCK:-}" = 1
python3 -B tests/autoscroll/ci_v5f_g3s_native_trace.py
