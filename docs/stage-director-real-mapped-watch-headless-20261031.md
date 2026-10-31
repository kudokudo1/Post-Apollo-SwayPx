# Stage Director SD1 // Real mapped Weather layer and WATCH protocol

STATE // Operator-approved Stage Director NEWS-first integration, draft isolated headless test.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director Weather Station native compositor integration (external Hospital-compatible).

ASSIGNMENT // Verify existing PR #41 native selection and PR #42 private WATCH/CLOSE against actual mapped layer-shell clients in full headless SwayPX.

PARENT // PR #42 head `4619470ac990cc1508a695b1953a0feb7bd907a2`; no merges or rebases authorized by this task.

## IMPLEMENTATION

This test reuses the existing headless SwayPX script and real layer-shell observer. It adds an independently compiled CI-only compositor with `-Dsd-ci-real-weather-watch-layer=true` (default **false**) and a separately compiled native Wayland client that creates an actual `zwlr_layer_surface_v1` with a SHM pixel buffer. The actual mapped Weather client uses its *same* `wl_display` to bind the newly published private `post_apollo_weather_watch_v1` protocol, issue WATCH, an invalid raw URL, CLOSE and a stale WATCH. A second Weather-namespace spoof surface lives on the opposite output edge and an unrelated layer also stays connected. Both must never discover the private global.

A CI-only guarded function invokes the compositor's genuine `stage_director_news_pairing_select_at()` scene hit-test at fixed coordinates (160,120), checking the mapped top-left 320x240 Weather layer, live native peer, role, output and epoch. This is **NOT** evidence of genuine physical input or operator authorization, only the actual native scene-to-protocol path. No synthetic key or pointer input is used. Normal SwayPX never compiles the CI-only selection hook.

The ordinary SwayPX build is independently compiled and scanned to ensure the CI marker isn't linked. It never launches the browser or grants compositor scene/input/popup authority. No Weather QML files are edited.

## HOW TO USE

Review the draft PR and its `Stage Director real mapped Weather WATCH layer (isolated headless)` GitHub Actions. Do not run this CI-only test binary on the operator workstation. The runner starts a fresh isolated headless compositor using pinned Nixpkgs, a private XDG_RUNTIME_DIR and separate software EGL paths, then terminates it.

## VERIFY

Require both normal and CI-only SwayPX builds, same-client real Wayland protocol WATCH/CLOSE, real mapped layer-shell with actual SHM buffer, native scene hit-test proof, duplicate-namespace spoof rejection, wrong-namespace rejection, source allowlist, CLOSE revocation, stale request denial, clean compositor shutdown. Both source guard and headless run must be green; log evidence printed from exact processes and runtime. Initial CI status is pending.

## WATCH OUT FOR

The headless compositor deliberately has no physical keyboard/mouse and cannot verify libinput versus uinput, genuine operator intent or a real click through a transparent Quickshell layer. The mock Weather layer is a small native C program, **not Qt/Quickshell**, so the same-connection Qt bridge remains a separate cross-worker requirement. Full NEWS video, browser spawning, scene crop, focus, popup and input transformation are not implemented. Real graphics drivers and multi-monitor scenarios also remain.

The CI-only layer callback is **compiled out by default** and additionally guarded by explicit GitHub CI environment variables. It may never become a production namespace-driven pairing shortcut. The bootstrap catalog IDs remain temporary fixtures.

## CHECKLIST

- [x] Recheck Syntagma, PR #42, HEAD and existing PR #36 headless real-layer harness
- [x] Reuse actual headless boot, pinned Mesa, and real layer-shell protocol generators
- [x] Add CI-only scene-hit selection (default off), no normal production bypass
- [x] Add native same-wl_client mapped Weather WATCH/CLOSE client
- [x] Test concurrent spoofed Weather and foreign layer clients
- [x] Add two-compositor-build + live headless CI workflow
- [ ] GitHub Actions green, correct any failure
- [ ] Qt/Quickshell bridge proof by Weather owner
- [ ] Real operator gesture, uinput hardening and cross-output selection
- [ ] Actual NEWS broker, clipping, popup/focus/input containment
- [ ] Operator-authorized local deployment and final acceptance

## NEXT

After headless success, coordinate the exact native Wayland connection and shared source ID catalog with Weather Station owner; begin broker/viewport slice only with independent security and render gates. If CI fails, preserve logs and repair the isolated test or real production seam. Do not conceal an unsupported assertion as a pass.

## DECISIONS

No local machine modifications, merges, real desktop activation, Weather Station ownership change or live Hospital registration. This is a draft test slice stacked on PR #42; production WATCH still has no viewer authority.
