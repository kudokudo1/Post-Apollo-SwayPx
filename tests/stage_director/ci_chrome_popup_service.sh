#!/usr/bin/env bash
# CI ONLY: run original Chrome plus a requested native XDG popup witness in
# ONE disposable, network-disabled systemd service cgroup. Never installed.
set -euo pipefail
if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || -z "${GITHUB_RUN_ID:-}" ||
      "${STAGE_DIRECTOR_CI_CHROME_POPUP_WITNESS:-}" != 1 ||
      "$#" -lt 4 ]]; then
  echo BLOCKED_POPUP_SERVICE_NOT_CI >&2
  exit 2
fi
witness="$1"
request="$2"
shift 2
expected="$RUNNER_TEMP/sd-ci-xdg-popup-$GITHUB_RUN_ID-${GITHUB_RUN_ATTEMPT:-1}.request"
if [[ "$request" != "$expected" || "$witness" != /nix/store/* ||
      ! -x "$witness" || "$1" != /opt/google/chrome/chrome ]]; then
  echo BLOCKED_POPUP_SERVICE_INVALID_SOURCE >&2
  exit 2
fi
"$@" &
chrome_pid=$!
for ((i=0; i<450; i++)); do
  if ! kill -0 "$chrome_pid" 2>/dev/null; then
    echo FAIL_POPUP_SERVICE_CHROME_EXITED >&2
    exit 1
  fi
  if [[ -f "$request" ]]; then
    if [[ "$(cat -- "$request")" != 'POPUP' ]]; then
      echo FAIL_POPUP_SERVICE_INVALID_REQUEST >&2
      exit 1
    fi
    "$witness" emit &
    witness_pid=$!
    echo "SD_CI_XDG_POPUP_SAME_CGROUP_WITNESS_STARTED pid=$witness_pid chrome_pid=$chrome_pid"
    break
  fi
  sleep 0.1
done
# Chrome retains the unit's lifetime; systemd kills the entire exact cgroup.
wait "$chrome_pid"
