# Stage Director SD1 // Real virtual keyboard/pointer delivery custody in isolated CI

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
WORKER // External Hospital-compatible Doctor, not verified live registered
BASE // `master` `495eb0ecce18eb383f968d8db6a8f20f3992bf6c`
STATE // Candidate; real input verification and merge outstanding

## IMPLEMENTATION

Merged PR #64 blocked `swaymsg [con_id=...] focus` for a hidden real Chrome sibling and kept the original visible Chrome native XDG focused, but **never sent an actual keyboard or pointer event**. Sway creates Wayland virtual-keyboard and virtual-pointer managers and routes their devices through its normal input manager. This CI-only experiment uses those existing public Wayland protocols, not a mock command or an unsupervised desktop injector.

Add a pinned, disposable native C client `tests/stage_director/ci_virtual_input_event.c`, built with the repository's flake-pinned Wayland scanner/wayland/xkbcommon. Copies of MIT-licensed wlroots virtual-keyboard and virtual-pointer protocol XML are retained with their licenses. The client refuses any environment except the exact CI runner and an explicit `STAGE_DIRECTOR_CI_CHROME_REAL_INPUT=1` gate. It sends a real virtual KEY_A press/release or a real absolute pointer motion on the same isolated compositor and native seat. It does not run on an operator workstation.

The compositor's `SD_STAGE_DIRECTOR_CI_SCENE_PROBE` build instruments **immediately before actual wlroots keyboard key and pointer motion notifications**. It classifies the focused/target root wl_surface as `MAIN`, `HIDDEN_SIBLING`, `OTHER` or `NONE`; this produces native event-delivery evidence, not title/app_id-based authority.

The seventh independent real-Chrome matrix lane `chrome-real-input` first repeats PR #64's visible Chrome positive and hidden Chrome negative IPC focus check. Then it:
- sends virtual KEY_A to the surviving focused Chrome main actor, requiring a real `KEYBOARD target=MAIN` delivery marker and no `target=HIDDEN_SIBLING`
- moves the virtual pointer into the clipped original Chrome viewport, requiring a real `POINTER target=MAIN` marker
- moves the pointer to the kernel-attested hidden sibling's own native rectangle, requiring **no** `POINTER target=HIDDEN_SIBLING` marker
- returns the pointer to the main viewport, verifies original native focus and geometry unchanged, then rechecks the real screenshot for sampled outside-mask leakage and complete owned service/profile/XDG teardown

## HOW TO USE

Review the GitHub PR > `Stage Director real Chrome stationary XDG moving mask (CI)` > `chrome-real-input` job. Look for `SD_CI_CHROME_REAL_INPUT: result=PASS`, `SD_CI_NATIVE_INPUT_DELIVERED` markers and existing `sd-ci-chrome-hidden-focus-denied.ppm/png` artifacts. No local installation required.

## VERIFY

- [ ] Pin and build native Wayland protocol emitter without compilation/runtime error
- [ ] Production compositor binary excludes input-target diagnostic
- [ ] CI compositor confirms actual keyboard and pointer delivery to root Chrome main actor
- [ ] No native root hidden sibling key or pointer notification after deliberate hidden focus/pointer attempt
- [ ] Real screenshot, identity, geometry and teardown proof remain green
- [ ] Existing six Chrome matrix cases and all other workflows pass on current head
- [ ] Merge and canonical recovery report only after verified CI

## WATCH OUT FOR

The client-origin input protocol is real, but `MAIN`/`HIDDEN_SIBLING` classification compares **root** wl_surfaces. Other nested Chrome `wl_subsurface` or popup surfaces are classified `OTHER`, not safely attributed. The tested pointer positions prove one explicit hit-test sequence, not arbitrary trajectories or pointer-button grabs. Native virtual input may require privileges on the isolated Wayland socket; do not bypass existing sandbox or production global filter. Actual keypress receipts in Chrome's JavaScript/event loop are not observed directly; this verifies compositor notification targets. Transient frame rendering, clipboard/data devices, real Weather NEWS pairing and production input broker remain unverified.

## CHECKLIST

- [x] Read canonical Syntagma and latest merged Stage Director state
- [x] Inspect existing wlroots virtual input managers and native delivery paths
- [x] Add pinned CI-only keyboard/pointer client with CI environment gate
- [x] Add CI-only event-target instrumentation near real keyboard/pointer notification
- [x] Add independent actual virtual-input matrix lane with positive and negative controls
- [ ] GitHub builds and native event verification
- [ ] PR merge and operator acceptance

## NEXT

On success, test input-event delivery for nested subsurface, real popup and pointer grabs with actual Wayland event witnesses; separately measure first-paint transient frames. Keep actual production NEWS launch default-deny pending trusted Weather host and operator approval.

## DECISIONS

No workstation, Weather Station QML, production input policy, or historical PR bulk merge. Bounded CI only, external Hospital-compatible worker, final authority retained by operator.
