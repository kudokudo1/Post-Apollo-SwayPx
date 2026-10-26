# Stage Director // Headless SwayPX runtime acceptance V0

**Patient:** `kudokudo1/Post-Apollo-SwayPx`  
**State:** Isolated, GitHub-runner-only runtime smoke, stacked on full-build PR #14.  
**Authority:** Operator final. No live Hospital Room/Assignment, local SwayFX activation or deployment implied.

## IMPLEMENTATION

The preceding full-build workflow compiled and linked the complete SwayPX compositor from the locked Nix flake after correcting SceneFX header dependencies in Stage Director's view/layer adapters. This is stronger than isolated C contracts, but compiling a binary does not prove it can start.

This step adds `tests/stage_director/headless_sway_smoke.sh` and a separate GitHub Action `.github/workflows/stage-director-headless-runtime.yml`. The workflow:

- Builds the full compositor from the exact checked-out PR revision using the repository's locked Nix flake.
- Starts **only** the built SwayFX binary with `WLR_BACKENDS=headless`, one virtual output, the software Pixman renderer and a private mode-0700 `XDG_RUNTIME_DIR` under the disposable GitHub runner's temporary directory.
- Uses its own minimal isolated Sway config. Does not read the operator's Sway, Quickshell, Weather Station or browser configuration.
- Waits for a real Wayland display socket and a still-running compositor, then requests termination of **only the direct child process started by the test**.
- Uses bounded startup/teardown loops, prints diagnostic logs on failure and cleans its temporary CI resources. Does not start a browser or interact with production machines.

## HOW TO USE

Inspect the **Stage Director isolated headless SwayPX runtime smoke** workflow attached to its stacked PR in GitHub. This is a GitHub-only acceptance check; no local commands or change to the desktop.

## VERIFY

A passing smoke test verifies only that the full SwayPX binary starts on a headless wlroots backend, presents its Wayland socket and terminates promptly under the test configuration. It does not prove a Weather Station layer is mapped, NEWS bay coordinates are correct, the native clip is activated, video is visible or mouse/keyboard/popups are isolated. The Weather NEWS viewer remains in safe mode.

**The result is not accepted until the exact workflow job completes successfully.** If software Pixman renderer or the CI environment is unsupported by SceneFX, preserve the failure and record that limitation rather than claiming the compositor code is defective or working.

## WATCH OUT FOR

- Headless + Pixman may behave differently from the operator's Wayland/NVIDIA graphics stack. A CI pass is not production deployment acceptance.
- A Wayland socket is a startup signal, not visual confirmation of actual clipping.
- No XDG popup surface, pointer, touch or keyboard event is created by this smoke.
- The test carefully kills only the process PID it spawned and uses a private temporary runtime; no broad `pkill`, host environment writes or startup from a taskbar control.
- The Stage Director browser launcher remains disabled and cannot be enabled by passing this test. The trusted session/popup/input/teardown boundary is still missing.
- This PR is stacked above PR #14; both depend on PR #13 and the SD0/SD1 development chain. The actual SwayPX `master` and live Fedora desktop remain distinct.

## CHECKLIST

- [x] Verify the complete SwayPX compiler/link workflow succeeded
- [x] Inspect actual Sway option parsing and headless backend availability
- [x] Add disposable, bounded Sway startup and stop test
- [x] Add separate isolated GitHub workflow and evidence
- [ ] Full headless startup smoke passes
- [ ] Validate real test actor paint, clip, focus, input and popup boundaries under an isolated compositor
- [ ] Operator accepts integration order and authorizes any eventual merge/deployment

## NEXT

After a successful headless startup check, create an isolated composited test actor in the NEWS viewport and verify **render clipping plus pointer/popup hit-test containment** against actual SwayPX event code, not only geometry probes. Keep browser process execution and the Weather Station WATCH path disabled until a fully trusted owner can enforce that boundary and clean up correctly.

## DECISIONS

Use the existing pinned Nix build and a narrowly scoped disposable headless backend. Keep this runtime smoke separate from the already-green full compiler check so startup/renderer limitations do not obscure compilation provenance.
