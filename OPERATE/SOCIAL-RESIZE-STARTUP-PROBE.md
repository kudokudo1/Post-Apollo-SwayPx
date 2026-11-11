# Social Resize // Persistent Startup Probe

**STATE:** opt-in diagnostic only. This does not alter the compositor source, the live Sway configuration, the existing `5414e450` GDM session, or the original `1cbca167` test entry.

## Why this exists

The compositor `1cbca167` compiled and passed `sway -C`, but three observed GDM logins showed a gray screen with a pointer. Existing GDM logs show compositor/IPC initialization without proof of Quickshell startup. The live config sends Quickshell output to `/tmp/quickshell.log`, which is overwritten by subsequent sessions.

The isolated launcher keeps a **per-login, private, persistent** log set, and launches the already-built compositor using the **same ordinary Sway configuration**, adding only `-d` to Sway arguments.

## Captured files

Per login, under `${XDG_STATE_HOME:-$HOME/.local/state}/swaypx/social-resize-startup/<UTC-time>-<PID>/`:

- `metadata.log`: session time, binary version, stage path, selected nonsecret session fields
- `sway.log`: complete compositor debug stdout/stderr
- `health.log`: approximately one-second Quickshell process presence and startup-log freshness samples
- `quickshell.log`: a periodically copied snapshot of `/tmp/quickshell.log`, **only after** it is modified in the observed session
- `exit.log`: compositor exit status and time **only if the shell is allowed to finish**
- `LATEST` at the parent directory: path of the most recent diagnostic run

A forced power-off or kernel hang might prevent the last buffers from reaching disk. An absent `exit.log` is not proof of a compositor crash. A Quickshell process from a different concurrent graphical session could also appear in `health.log`; inspect session identity before treating presence as proof that the requested desktop loaded.

The directory uses restrictive user permissions (`umask 077`), but the application logs can still contain private data. Review or redact before sharing.

## Prepare a *new* optional GDM entry on the Fedora host

Only after the operator authorizes installation, acquire `OPERATE/social-resize-startup-probe.sh` from this reviewed revision. Copy it to:

```text
~/.local/opt/swaypx-staging/1cbca167/bin/swaypx-social-resize-probe
```

Give that copy executable permission. Check that the staged Sway binary still exists and the launcher passes `sh -n`.

Create a *separate* GDM session file (do not edit either original test entry):

```ini
[Desktop Entry]
Name=SwayPX - Social Resize Diagnostic (1cbca167)
Comment=Persistent SwayPX and Quickshell startup diagnostics
Exec=/var/home/mapple/.local/opt/swaypx-staging/1cbca167/bin/swaypx-social-resize-probe
TryExec=/var/home/mapple/.local/opt/swaypx-staging/1cbca167/bin/swaypx-social-resize-probe
Type=Application
DesktopNames=sway;wlroots
```

Use a distinct filename, e.g. `/usr/local/share/wayland-sessions/swaypx-social-resize-diagnostic-1cbca167.desktop`. The account path above is **specific to the observed Fedora host**, not portable configuration and not installed by this PR.

The script itself has no installer. GDM registration and any later login into the diagnostic session are separate, operator-approved local actions. Never change the current live Sway config to capture this test.

## Read after returning to the working desktop

```bash
ROOT="${XDG_STATE_HOME:-$HOME/.local/state}/swaypx/social-resize-startup"
RUN="$(/usr/bin/cat "$ROOT/LATEST")"
printf 'Probe run: %s\n' "$RUN"
sed -n '1,60p' "$RUN/metadata.log"
tail -80 "$RUN/sway.log"
tail -40 "$RUN/health.log"
if [ -f "$RUN/quickshell.log" ]; then
    tail -80 "$RUN/quickshell.log"
else
    printf 'No current Quickshell log was captured\n'
fi
if [ -f "$RUN/exit.log" ]; then
    sed -n '1,20p' "$RUN/exit.log"
fi
```

## Verify and rollback

Run the self-contained offline test:

```bash
python3 tests/stage_director/test_social_resize_startup_probe.py
```

It uses a disposable mock Sway executable, never opens a Wayland session, and asserts timestamped logs, exit propagation, 0700 storage, and stale-log rejection. **A passing mock test does not prove the actual Fedora/GDM session works.**

The prior `5414e450` and `c3b2f6db` entries remain untouched. If the diagnostic session is unusable, select `5414e450` at GDM. Later, with separate approval, remove **only** the newly installed diagnostic entry and wrapper; leave its logs as evidence until the operator authorizes removal.
