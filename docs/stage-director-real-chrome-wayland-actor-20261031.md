# Stage Director SD1 // Real Chrome Wayland actor provenance in isolated SwayPX

STATE // Draft isolated verification only; production WATCH cannot start browsers.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director Weather Station NEWS viewer, Hospital-compatible external worker only.

ASSIGNMENT // Verify real Chrome maps a native xdg_toplevel inside an independently booted SwayPX compositor and matches the exact kernel process identity within its supervised systemd cgroup.

BASE // PR #47 head `ac45085669511590f61f94076d86ee781c1efff6`; ancestors #43 publication security fix and #45 single WATCH lease required.

OWNERSHIP // SwayPX integration only; Weather Station QML / Qt bridge has a separate owner.

## IMPLEMENTATION

This CI-only test reuses the existing SwayPX isolated software-EGL headless compositor and temporarily managed /dev/udmabuf ACL. It adds an explicit guarded step to launch installed real Chrome (Chromium-family) on the **isolated headless compositor's actual Wayland socket** with `--ozone-platform=wayland` and fixed `--app=about:blank`. The browser is placed in a restricted transient **systemd service** with a private network, non-root runner account, cgroup v2, private `RuntimeDirectory` profile, `NoNewPrivileges`, protected system, read-only host home and `KillMode=control-group`.

The test compares Sway's native `xdg_shell` mapped top-level view PID with the members of the independently owned service cgroup and requires the kernel starttime to match. Window title/app_id cannot be treated as native provenance. It refuses non-unique windows, stale PID/starttime, missing shell role, nonexistent native Wayland socket and insufficient real Chrome subprocesses.

Once confirmed, CI stops the exact service unit, requires the recorded process identities and cgroup members to disappear, verifies the private profile directory is removed, and requires the Sway native window to be unmapped. All cleanup is bounded and always executed on failure.

**This is not a production browser launch issuer**, clip, fullscreen, focus, keyboard or mouse forwarding implementation. It does not use NEWS WATCH to start the test browser, nor change its current candidate-only semantics. The native Sway observer and compositor session/process policies already exist, so this slice tests their shared boundary instead of duplicating them.

## HOW TO USE

Review draft PR and `Stage Director real Chrome native Wayland actor (CI only)` GitHub Actions. Everything runs on a disposable GitHub Ubuntu runner. No commands should be run on the operator Fedora desktop.

## VERIFY

Require source isolation check and full production SwayPX binary with no automatic CI-only scene pairing selector; independently start isolated compositor, verify Chrome systemd-cgroup + mapped exact native `xdg_shell` PID/starttime and observable geometry; stop and observe complete root/descendant cleanup plus actor unmap and /dev/udmabuf ACL restoration. GitHub CI pending at creation.

## WATCH OUT FOR

- The process identity correlation is **read-only**. A mapped Chrome actor is not allowed to render in a Weather viewport; popup, focus, input, crop and routing are still separate barriers.
- A CI service uses `ProtectHome=read-only` rather than `ProtectHome=yes` because the compositor's private Wayland socket lives under the GitHub runner's HOME. This specific isolation compromise remains CI-only with no external network and fixed `about:blank`; production must use an independent private runtime location and stronger path isolation.
- Chrome sandbox internal AppArmor/namespaces are not attested simply because no `--no-sandbox` flag appears.
- The earlier Docker Chromium sandbox test remains blocked by namespace EPERM and is a separate manual negative diagnostic. The systemd test uses the runner's installed Chrome.
- Browser cgroup custody is currently managed by systemd in the runner, NOT the production SwayPX session, which still has no trusted browser launch issuer.
- The actual Weather Quickshell panel/Qt bridge and its mutable viewport are not included; separate worker owns them.
- Do not merge PR #42 without its security-correcting descendant PR #43.

## CHECKLIST

- [x] Verify canonical Syntagma, present PR stack, existing XDG native observer / process lineage and clip foundations
- [x] Reuse headless SwayPX and real Chrome systemd cgroup test approach
- [x] Add read-only real Chrome native XDG / kernel process identity test
- [x] Require full service group cleanup, disappearance of mapped Wayland view and bounded teardown
- [x] Add GitHub CI and source authority guards
- [ ] GitHub real run pass or precise failure diagnostics
- [ ] Actual host browser session connection to authenticated WATCH
- [ ] Clip/SceneFX paint, input/focus and popup containment under concurrent browser surfaces
- [ ] Actual Weather Quickshell bridge integration and operator acceptance

## NEXT

If this CI passes, implement a separately approved compositor-owned broker/launcher with exact native cgroup and process identity, and make the viewport clip and popup/input guard enforceable in the same live session before granting production WATCH browser activation. If failure is a runner/Wayland mapping incompatibility, preserve logs and do not bypass the sandbox.

## DECISIONS

User permitted merges when necessary, but no merge is needed to test this stacked branch. No live Hospital registration, local workstation modification, Weather QML edit, browser playback or production activation is claimed.
