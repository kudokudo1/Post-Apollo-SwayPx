#!/usr/bin/env bash
# GitHub-hosted ephemeral runner only. Never use on the operator's desktop.
# Device access is granted to this unprivileged CI user for one test and
# restored immediately. No production compositor or allocator modification.
set -euo pipefail

sway_binary="${1:?pass exact Nix-built compositor path}"
device=/dev/udmabuf
uid="$(id -u)"
acl_changed=0

cleanup() {
  local rc=$?
  trap - EXIT
  if [[ "$acl_changed" == 1 ]]; then
    if sudo -n setfacl -x "u:$uid" "$device"; then
      echo "Stage Director CI: removed temporary /dev/udmabuf user ACL"
    else
      echo "::error title=UDMABUF_ACL_RESTORE_FAILED::Could not restore temporary runner device ACL" >&2
      rc=3
    fi
  fi
  exit "$rc"
}
trap cleanup EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" ]]; then
  echo "BLOCKED_NOT_GITHUB_RUNNER: refuse device permission changes outside CI" >&2
  exit 2
fi

if [[ ! -c "$device" || ! -x "$sway_binary" ]]; then
  echo "BLOCKED_UDMABUF_DEVICE_OR_BINARY: missing character device or built SwayPX" >&2
  exit 2
fi

echo "Stage Director CI device: $(stat -Lc '%n mode=%a owner_uid=%u group_gid=%g major=%t minor=%T' "$device")"
if [[ "$(stat -Lc '%u' "$device")" != 0 ]]; then
  echo "BLOCKED_UDMABUF_OWNER: unexpected non-root-owned device; refusing modifications" >&2
  exit 2
fi

can_open_device() {
  python3 -c 'import os; fd=os.open("/dev/udmabuf", os.O_RDWR | os.O_CLOEXEC); os.close(fd)' \
    >/dev/null 2>&1
}

if can_open_device; then
  echo "Stage Director CI: /dev/udmabuf already accessible to test user"
else
  # Do not run Sway as root, use chmod 666, or change a pre-existing ACL.
  if ! command -v setfacl >/dev/null 2>&1 ||
      ! command -v getfacl >/dev/null 2>&1; then
    echo "BLOCKED_ACL_TOOL_UNAVAILABLE: cannot grant narrow device access" >&2
    exit 2
  fi
  if getfacl -cp "$device" | grep -Eq "^user:$uid:"; then
    echo "BLOCKED_UDMABUF_PREEXISTING_ACL: preserving existing ACL and refusing overwrite" >&2
    exit 2
  fi
  if ! sudo -n setfacl -m "u:$uid:rw" "$device"; then
    echo "BLOCKED_UDMABUF_ACL_DENIED: ephemeral runner cannot grant device access" >&2
    exit 2
  fi
  acl_changed=1
  if ! can_open_device; then
    echo "BLOCKED_UDMABUF_KERNEL_ACCESS: ACL applied but kernel still denies open" >&2
    exit 2
  fi
  echo "Stage Director CI: granted temporary uid=$uid rw ACL to /dev/udmabuf"
fi

# The compositor is still the ordinary CI user. The existing smoke script
# creates a private runtime, starts precisely one headless process, and
# terminates/reaps it. Cleanup above removes our ACL even on test failure.
bash tests/stage_director/headless_sway_smoke.sh "$sway_binary"
