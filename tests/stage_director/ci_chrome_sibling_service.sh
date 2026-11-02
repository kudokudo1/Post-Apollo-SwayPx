#!/usr/bin/env bash
# CI ONLY: second native window requested by real Chrome in the same
# disposable systemd service/profile. Not production browser authority.
set -euo pipefail
if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      "${STAGE_DIRECTOR_CI_CHROME_SIBLING_DISCOVERY:-}" != 1 ||
      -z "${GITHUB_RUN_ID:-}" || -z "${RUNNER_TEMP:-}" ||
      "$RUNNER_TEMP" != /home/runner/work/_temp || "$#" -lt 4 ]]; then
  echo BLOCKED_CHROME_SIBLING_NOT_DISPOSABLE_CI >&2
  exit 2
fi
request="$1"
shift
run_id="$GITHUB_RUN_ID-${GITHUB_RUN_ATTEMPT:-1}"
expected="$RUNNER_TEMP/sd-ci-chrome-sibling-$run_id.request"
expected_profile="/run/sd-ci-chrome-wayland-$run_id/profile"
if [[ "$request" != "$expected" || "$1" != /opt/google/chrome/chrome ]]; then
  echo BLOCKED_CHROME_SIBLING_REQUEST_OR_EXECUTABLE >&2
  exit 2
fi
profile_arg=""
for a in "$@"; do
  if [[ "$a" == --user-data-dir=* ]]; then profile_arg="${a#--user-data-dir=}"; fi
done
if [[ "$profile_arg" != "$expected_profile" ]]; then
  echo BLOCKED_CHROME_SIBLING_FOREIGN_PROFILE >&2
  exit 2
fi
"$@" &
chrome_root=$!
# Multi-mode staging retains the exact disposable Chrome service/profile.
# Each new request is separately bounded and has exact contents.
stages=1
if [[ "${STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING:-}" == 1 ]]; then stages=3; fi
for ((stage=1; stage<=stages; stage++)); do
  path="$expected"
  if ((stage > 1)); then path="$expected.$stage"; fi
  observed=0
  for ((i=0; i<400; i++)); do
    if ! kill -0 "$chrome_root" 2>/dev/null; then
      echo FAIL_CHROME_SIBLING_ORIGINAL_EXITED >&2
      exit 1
    fi
    if [[ -e "$path" ]]; then
      if [[ ! -f "$path" || -L "$path" ]]; then
        echo FAIL_CHROME_SIBLING_REQUEST_NOT_REGULAR >&2
        exit 1
      fi
      required="SIBLING"
      if ((stage > 1)); then required="SIBLING$stage"; fi
      if [[ "$(cat -- "$path")" != "$required" ]]; then
        echo FAIL_CHROME_SIBLING_REQUEST_BYTES >&2
        exit 1
      fi
      /opt/google/chrome/chrome \
        --ozone-platform=wayland \
        --disable-gpu --disable-background-networking \
        --disable-component-update --disable-extensions --disable-sync \
        --no-first-run --no-default-browser-check \
        "--user-data-dir=$expected_profile" \
        --new-window about:blank &
      extra_invocation=$!
      echo "SD_CI_CHROME_SIBLING_EXECUTABLE_REQUESTED stage=$stage real_chrome=1 same_private_profile=1 same_service_cgroup=1 launcher_pid=$extra_invocation original_pid=$chrome_root authority=NONE"
      observed=1
      break
    fi
    sleep 0.1
  done
  if ((observed != 1)); then
    echo "FAIL_CHROME_SIBLING_STAGE_REQUEST_TIMEOUT stage=$stage" >&2
    exit 1
  fi
done
wait "$chrome_root"
