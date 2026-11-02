# Stage Director SD1 // Abrupt real Chrome exit and scene-pixel recovery

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Weather Station / Stage Director NEWS viewer
ASSIGNMENT // CI-only native Chrome forced termination / stale-pixel recovery proof
WORKER // External Hospital-compatible ChatGPT doctor, not live Hospital-registered
BASE // `master` `c04fab87a945e17f8c38b8075f1058570344a901`
STATE // Source candidate, GitHub CI and operator review outstanding

## IMPLEMENTATION

Extend PR #56's actual Chrome three-state mask test. The browser is first rendered and verified through the initial 320x240, moved 400x300, and moved/shrunk 280x220 mask. Then the same test *deliberately* sends SIGKILL to the **exact isolated Chrome systemd service created by this runner**, with `--kill-whom=all`. The runner is locked to GitHub Actions, one disposable Wayland socket/runtime, an exact known private service cgroup, a fixed `about:blank` URL and a network-disabled browser sandbox unit. It does not kill arbitrary processes, look up browser windows by title, or invoke any production WATCH pathway.

Before screenshot capture, the harness independently checks all held process generations have died, the exact service cgroup is empty, the Chrome native XDG window is gone from the live Sway IPC tree, and the CI-only `stage_director_ci_scene_probe_restore()` unmap marker appeared after SIGKILL. A new real grim PPM + PNG is taken **after** the abrupt exit. The pixel checker compares bright third-mask pre-kill pixels to black post-kill pixels across the final viewport, both prior viewports and all four surrounding edges. It checks equality to the pre-Chrome black baseline within a small tolerance and fails if any stale pixels remain.

This experiment reuses the existing compositor view-unmap lifecycle hook. No new production C or QML file is needed; no live browser spawn/clip/seat/popup authority is introduced.

## HOW TO USE

Review GitHub PR / Actions: `Stage Director real Chrome stationary XDG moving mask (CI)`, job `real-chrome-four-edge-pixels`. Inspect the post-kill pixel log and retained screenshots named `sd-chrome-abrupt-exit.*`. No operator desktop steps.

## VERIFY

- [ ] GitHub source-boundary and real-Chrome screenshot workflow pass
- [ ] Full normal production SwayPX binary excludes CI scene selector
- [ ] Correct SIGKILL service and surviving compositor/native Wayland IPC observed
- [ ] Exact native XDG unmapped; browser process generations and cgroup cleared
- [ ] Post-abort actual screenshot matches black pre-Chrome background
- [ ] Other Stage Director workflows pass; no unrelated files touched

## WATCH OUT FOR

This proves the post-PHASE3 **abnormal actor termination**, not an interrupted transaction exactly between frames. During-transaction kill, compositor crash resilience, multi-surface popups, pointers, keyboard, focus, and inner NEWS native geometry attestation still require separate evidence. GitHub CI does not imply live Fedora activation or final operator acceptance. Historical PR #51 outer window move remains blocked.

## CHECKLIST

- [x] Verify #56 merged default branch and canonical Syntagma protocols
- [x] Inspect actual CI restore on `stage_director_view_unmapped()`
- [x] Add fixed-identity SIGKILL and post-exit screenshot path
- [x] Add checker for final mask, previous masks, four edges, native XDG and black baseline
- [x] Add source safeguards and CI screenshot artifacts
- [ ] Pass all triggered GitHub workflows and merge
- [ ] Continue earlier/interrupted-mask kill conditions
- [ ] Coordinate actual inner NEWS geometry with Weather QML owner
- [ ] Verify popup/input/focus confinement before any active viewer

## NEXT

After CI passes, move to **kill timing coverage at PHASE2 and while a request is outstanding** rather than treating a single forced exit after PHASE3 as complete crash safety. Continue separate compositor-authenticated inner NEWS surface provenance and browser popup/input guards.

## DECISIONS

Only the exact disposable Chrome service from the existing isolated harness may be SIGKILLed. Never invoke production browser, change the operator's local Fedora session, cross the independent Weather Station QML ownership boundary, or bulk-merge historic stacked PRs.
