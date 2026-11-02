# Stage Director SD1 // Real XDG popup positive/negative pixel proof

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // Native popup protocol and rendered-pixel evidence for CI popup denial
WORKER // External Hospital-compatible ChatGPT doctor; live Hospital registration unverified
BASE // `master` `4dacd37d0d6abf05b7eceb0e4009107a3f466a58`
STATE // Implementation candidate, GitHub CI pending

## IMPLEMENTATION

Build a disposable, pinned, native Wayland XDG popup emitter with `wayland-scanner`. Its parent is a real `xdg_toplevel` with a 24x24 black SHM buffer. Its child is a real `xdg_popup` carrying a 160x100 solid RGB DD44DD SHM buffer. Buffers use Linux anonymous `memfd_create`, so the witness can run under Chrome's existing `ProtectHome=read-only` service without weakening the sandbox. The emitter is a CI test executable, not Chrome or a Weather Station client.

Run two **different native process ancestries** against the same isolated SwayPX compositor:
1. Positive control: launch the witness directly under the unrelated runner process/cgroup. This native `xdg_popup` must map, attach the magenta SHM buffer and yield at least 800 actual magenta screenshot pixels.
2. Negative case: launch the identical emitter as a process descendant of the exact disposable, network-isolated systemd service already running actual Chrome. A fixed, 0600 request file triggers a tightly bounded service wrapper, which does not expose any new production action. The existing PR #59 CI popup hook must record `SD_CI_CHROME_POPUP_DENIED` with `current_cgroup=1` before attaching a global popup scene. The native witness must have actually flushed `xdg_surface_get_popup` to the compositor, and a new screenshot must contain at most 10 magenta pixels.

Real screenshots `sd-ci-native-popup-positive.ppm/.png` and `sd-ci-native-popup-denied.ppm/.png` plus C emitter/service logs are retained as GitHub Actions artifacts. A third `native-popup-denial` matrix variant is isolated from the pre-existing `after-third-mask` and `pending-before-move` runs. It also executes the existing Chrome mask/abrupt-exit regression in the same exact service.

## HOW TO USE

Review the PR's `Stage Director real Chrome stationary XDG moving mask (CI)` workflow, matrix job `native-popup-denial`. Inspect `SD_CI_NATIVE_POPUP_DENIAL_PIXELS: result=PASS` and `SD_CI_XDG_POPUP_NATIVE_EVENT` if green. No operator workstation changes or manual reproduction.

## VERIFY

- [ ] Positive unrelated native `xdg_popup` protocol created, mapped and actually paints DD44DD in a real screenshot
- [ ] Same emitter, different exact Chrome service cgroup, sends native popup request
- [ ] Compositor hook logs actual `SD_CI_CHROME_POPUP_DENIED` with `current_cgroup=1`
- [ ] Denied screenshot has no DD44DD popup pixels, despite successful control
- [ ] Real Chrome process/cgroup and global mask teardown still pass
- [ ] Source gate, full production SwayPX build and both existing Chrome regressions remain green
- [ ] Merge only with all checks passing and Git HEAD/ownership rechecked

## WATCH OUT FOR

The native emitter is NOT the Chrome executable itself. It is an independent native Wayland client deliberately run *inside Chrome's exact disposable service cgroup* so the compositor exercises the already-implemented native cgroup admission predicate. Therefore success would prove **a real popup event and observable deny behavior for same-cgroup clients**, but not that Chrome generated its own popup or that all browser popup surfaces are contained. In particular, separate XDG toplevel windows, nested real Chrome popups, independently spawned clients and subsurfaces remain open. Popups from unrelated native applications must not be affected.

This still does not authorize production browser launch, popup display, pointer/keyboard/focus, NEWS QML connection, host viewport integration or Chrome internal sandbox attestation.

## CHECKLIST

- [x] Inspect Syntagma and verify #59 merged/current master
- [x] Reuse exact Chrome service, native popup gate and existing pixel capture
- [x] Add native XDG popup emitter and pinned Nix derivation
- [x] Add independent real popup pixel positive/negative checker
- [x] Preserve the Chrome service sandbox with an exact request wrapper
- [x] Add third isolated GitHub Actions matrix variant
- [ ] GitHub CI proof and merge
- [ ] Actual Chrome-generated popup trial
- [ ] Additional XDG windows/subsurface escape and focus/seat routing trials
- [ ] Operator desktop acceptance

## NEXT

If real native popup evidence passes, proceed to actual Chromium XDG popup generation, then distinct toplevel/subsurface probes. Do not confuse a witness deliberately placed in the Chrome service cgroup with the Chrome browser emitting the popup itself. Recheck live ownership before crossing Weather Station QML.

## DECISIONS

CI-only source and artifacts; no local Fedora write, production activation or mass merge of historical PRs. Preserve operator final acceptance authority.
