#!/bin/sh
# Isolated diagnostic GDM launcher for an already-built, opt-in SwayPX stage.
# No modifications to the live Sway configuration or Quickshell startup rule.
set -u
umask 077

if [ -z "${HOME:-}" ]; then
    printf 'social-resize-probe: HOME is unset\n' >&2
    exit 78
fi

stage=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P) || exit 78
sway="$stage/bin/sway"
if [ ! -x "$sway" ]; then
    printf 'social-resize-probe: compositor not executable: %s\n' "$sway" >&2
    exit 127
fi

state_base=${XDG_STATE_HOME:-"$HOME/.local/state"}
log_root="$state_base/swaypx/social-resize-startup"
if ! mkdir -p -- "$log_root"; then
    printf 'social-resize-probe: cannot create private log directory: %s\n' "$log_root" >&2
    exit 73
fi

run="$log_root/$(date -u +%Y%m%dT%H%M%SZ)-$$"
if ! mkdir -- "$run"; then
    printf 'social-resize-probe: cannot create run directory\n' >&2
    exit 73
fi
printf '%s\n' "$run" > "$log_root/LATEST"

export LD_LIBRARY_PATH="$stage/lib:$stage/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
quickshell_log=${SOCIAL_RESIZE_PROBE_QUICKSHELL_LOG:-/tmp/quickshell.log}
started_epoch=$(date +%s)

{
    printf 'run=%s\nstart=%s\nstage=%s\n' "$run" "$(date -Is)" "$stage"
    printf 'uid=%s\n' "$(id -u)"
    printf 'XDG_SESSION_TYPE=%s\nXDG_CURRENT_DESKTOP=%s\n' \
        "${XDG_SESSION_TYPE:-unset}" "${XDG_CURRENT_DESKTOP:-unset}"
    printf 'WAYLAND_DISPLAY=%s\nDISPLAY=%s\n' \
        "${WAYLAND_DISPLAY:-unset}" "${DISPLAY:-unset}"
    "$sway" --version
} > "$run/metadata.log" 2>&1

# Copy only a log written in this invocation's time window. The normal Sway
# config still writes /tmp/quickshell.log; no shared config is changed.
(
    while :; do
        printf '%s ' "$(date -Is)"
        if command -v pgrep >/dev/null 2>&1; then
            pgrep -a -u "$(id -u)" -x quickshell || printf 'quickshell=absent\n'
        else
            printf 'pgrep=unavailable\n'
        fi
        if [ -f "$quickshell_log" ]; then
            log_mtime=$(stat -c %Y -- "$quickshell_log" 2>/dev/null || printf '0')
            if [ "$log_mtime" -ge "$started_epoch" ]; then
                cp -- "$quickshell_log" "$run/quickshell.log" 2>/dev/null || :
                printf 'quickshell_log=current mtime=%s\n' "$log_mtime"
            else
                printf 'quickshell_log=older_than_probe mtime=%s\n' "$log_mtime"
            fi
        else
            printf 'quickshell_log=absent\n'
        fi
        sleep 1
    done
) >> "$run/health.log" 2>&1 &
monitor_pid=$!
sway_pid=

cleanup() {
    if [ -n "${monitor_pid:-}" ]; then
        kill "$monitor_pid" 2>/dev/null || :
        wait "$monitor_pid" 2>/dev/null || :
    fi
}
terminate() {
    trap - HUP INT TERM
    if [ -n "${sway_pid:-}" ]; then
        kill -TERM "$sway_pid" 2>/dev/null || :
        wait "$sway_pid" 2>/dev/null || :
    fi
    exit 143
}
trap cleanup 0
trap terminate HUP INT TERM

# Run the compositor with the SAME normal user configuration as the working
# session. Debug logging is the only extra Sway argument.
"$sway" -d "$@" > "$run/sway.log" 2>&1 &
sway_pid=$!
wait "$sway_pid"
result=$?
printf 'exit=%s\ntime=%s\n' "$result" "$(date -Is)" > "$run/exit.log"
exit "$result"
