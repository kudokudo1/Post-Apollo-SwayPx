#!/usr/bin/env bash
# CI-only native SwayPX real-pair test: allow /dev/udmabuf temporarily.
# This mirrors the existing Stage Director device guard, with its own
# test entrypoint. NEVER run on the operator's workstation.
set -euo pipefail

device=/dev/udmabuf
acl_changed=0
acl_snapshot=""
uid="$(id -u)"

cleanup() {
  local rc=$?
  trap - EXIT
  if [[ "$acl_changed" == 1 ]]; then
    if sudo -n setfacl --restore="$acl_snapshot"; then
      echo "SD_SOCIAL_CI_UDMABUF original_acl=RESTORED"
    else
      echo "::error title=SOCIAL_UDMABUF_ACL_RESTORE_FAILED::Temporary runner device ACL not restored" >&2
      rc=3
    fi
  fi
  if [[ -n "$acl_snapshot" ]]; then
    rm -f -- "$acl_snapshot"
  fi
  exit "$rc"
}
trap cleanup EXIT

if [[ "${GITHUB_ACTIONS:-}" != true ||
      "${CI:-}" != true ||
      "${STAGE_DIRECTOR_CI_SOCIAL_REAL_PAIR:-}" != 1 ||
      "${STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE:-}" != 1 ||
      "${RUNNER_TEMP:-}" != /home/runner/work/_temp ||
      "$uid" == 0 ||
      "${SD_SOCIAL_CI_SWAY:-}" != /nix/store/* ]]; then
  echo "BLOCKED_SOCIAL_UDMABUF_NOT_EXACT_DISPOSABLE_CI" >&2
  exit 2
fi
if [[ ! -c "$device" || ! -x "$SD_SOCIAL_CI_SWAY" ||
      "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "BLOCKED_SOCIAL_UDMABUF_DEVICE_OWNER_OR_BINARY" >&2
  exit 2
fi

can_open_device() {
  python3 -c 'import os; f=os.open("/dev/udmabuf", os.O_RDWR | os.O_CLOEXEC); os.close(f)' \
    >/dev/null 2>&1
}

if ! can_open_device; then
  if ! command -v setfacl >/dev/null 2>&1 ||
      ! command -v getfacl >/dev/null 2>&1; then
    echo "BLOCKED_SOCIAL_UDMABUF_ACL_TOOLS_MISSING" >&2
    exit 2
  fi
  if getfacl -cp "$device" | grep -Eq "^user:$uid:"; then
    echo "BLOCKED_SOCIAL_UDMABUF_PREEXISTING_USER_ACL" >&2
    exit 2
  fi
  acl_snapshot="$(mktemp "$RUNNER_TEMP/social-udmabuf-acl.XXXXXXXX")"
  getfacl -p "$device" > "$acl_snapshot"
  acl_changed=1
  if ! sudo -n setfacl -m "u:$uid:rw" "$device"; then
    echo "BLOCKED_SOCIAL_UDMABUF_CI_ACL_GRANT_DENIED" >&2
    exit 2
  fi
  if ! can_open_device; then
    echo "BLOCKED_SOCIAL_UDMABUF_STILL_DENIED" >&2
    exit 2
  fi
  echo "SD_SOCIAL_CI_UDMABUF temporary_acl=GRANTED owner=CI_ONLY"
else
  echo "SD_SOCIAL_CI_UDMABUF access=ALREADY_PRESENT no_acl_change=1"
fi

# The only compositor launched is inside the Python harness' private CI
# runtime. It reaps clients/sway before returning; trap restores device ACL.
python3 -B tests/stage_director/ci_social_production_native_runtime.py
