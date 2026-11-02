# Stage Director SD1 // Real hidden Chrome seat-focus custody (isolated CI)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
PROVIDER // OPEN AI // CHATGPT; external Hospital-compatible, no verified live Hospital session
ASSIGNMENT // Block native focus of an invisible Chrome sibling independent of scene paint
BASE // `master` `b0ab9b0a7cb48840acd0b375a5ecce932e93dbdb`
STATE // CI-only candidate; verification and operator acceptance pending

## IMPLEMENTATION

PR #63 proved two simultaneously mapped real Chrome sibling XDG windows can be made visually invisible in an isolated SwayPX compositor, with exact close/reopen and bounded scene-slot recycling. It did NOT prove that keyboard focus follows those scenes. Sway's `seat_set_workspace_focus()` and `seat_set_focus_surface()` independently determine the destination for native Wayland activation and keyboard enter. A disabled scene is **not** inherently a focus denial.

This narrow CI-only change links a native view focus guard through the preexisting optional `SD_STAGE_DIRECTOR_CI_SCENE_PROBE` build and exact Chrome service cgroup source:
- `sway/input/seat.c` rejects a kernel/cgroup-authenticated **sibling** before changing the previous focus, seat focus stack, native keyboard enter, IM relay or pointer constraint.
- The direct `seat_set_focus_surface` path also rejects the exact root surface of any tracked hidden sibling before clearing existing focus. Nested subsurface focus remains a separate unproven path.
- No title, app_id, URL or window name is an authority source. The visible main Chrome actor and unrelated apps continue through normal Sway focus. The production binary has no test-only focus selector.

The separate `chrome-hidden-focus` GitHub matrix job uses the **real Chrome executable**, exact disposable private service/profile and the previous second-window test. It requires both live native XDG IDs and kernel cgroup/proc identity. A Sway IPC focus command to the **visible** original native ID is a positive control; the same type of focus command to the hidden sibling ID must not move active native focus or geometry. The compositor must log its exact denied seat path, and a real screenshot after the attempt must retain zero sampled bright pixels outside the original viewport and stable original geometry.

The initial PR-head Chrome screenshot CI run had two unrelated pre-browser startup IPC response failures: `swaymsg get_tree` could not receive an IPC response while the disposable compositor initialized, **before the Chrome session or focus test was launched**. Two independent retry attempts of the focus lane reproduced the same early failure. The unrelated titlebar control now performs at most **four explicitly logged get_tree retries**, and only for this pre-Chrome startup observation. Later browser geometry, Chrome PID/cgroup provenance, focus-negative, screenshot and teardown calls retain strict no-retry behavior. Exhaustion fails closed. The exact cause of the initial IPC responses is not yet verified, and adding a bounded retry does not represent a renderer/security fix.

## HOW TO USE

Review PR checks > `Stage Director real Chrome stationary XDG moving mask (CI)` > `chrome-hidden-focus`. The screenshot `sd-ci-chrome-hidden-focus-denied.ppm/png` and Sway IPC tree are artifacts. No local Fedora/Quickshell work.

## VERIFY

- [ ] Source gate and ordinary production SwayPX binary still build without CI seat guard
- [ ] Real Chrome initial + sibling native XDG identity confirmed in exact owned service
- [ ] IPC positive focus test succeeds for visible main Chrome
- [ ] Exact IPC request to hidden sibling is denied by native seat gate; focus stays original
- [ ] Real post-attempt PPM has zero sampled bright outside-mask points and unchanged geometry
- [ ] Existing popup, multi-XDG, mask/exit/headless checks remain green, no regressions
- [ ] PR merge only after current-head workflow and exact master checks

## WATCH OUT FOR

Sway IPC targeting the native container tests a real Sway seat-focus path. No physical or virtual keyboard device is attached in this headless CI compositor (`WLR_LIBINPUT_NO_DEVICES=1`), so this does **not** prove individual Wayland key events cannot reach a hidden window by an independent injection path. Direct root-surface focus has a source guard but no independent event-driven runtime proof yet. Pointer hover/click, touch, data devices, descendant/subsurface focus, popups and native session broker still require separate tests. A hidden scene with blocked focus is not production input authorization.

## CHECKLIST

- [x] Recover PR #63 merged master and read Syntagma/seat source
- [x] Add CI-only native sibling rejection before workspace focus or direct root focus changes
- [x] Add real Chrome positive/negative exact-native-ID IPC focus and screenshot test
- [x] Preserve existing independent Chrome and production checks
- [ ] Green GitHub CI and merge
- [ ] Actual pointer/keyboard-event provenance and nested subsurface tests
- [ ] Real Weather Station NEWS host and operator acceptance

## NEXT

After a successful bounded focus test, separately verify actual pointer/keyboard protocol events under an isolated Wayland virtual input device or disposable client (no live desktop, permission or production change). Independently verify transient first-paint containment and attested Weather NEWS inner viewport geometry.

## DECISIONS

No local Fedora, Weather Station QML, or production browser activation. No automatic promotion of CI-only seat policy to production. No historical PR mass merges.
