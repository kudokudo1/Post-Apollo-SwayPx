# Stage Director SD1 // CI-only multiple real Chrome native XDG sibling lifecycles

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
WORKER // External Hospital-compatible Doctor (no verified live registration)
BASE // `master` `351695ce68f18642684f53bc78a0ba7643766dcb` (merged #62)
STATE // CI candidate, verification/merge outstanding
ASSIGNMENT // Two simultaneous Chrome sibling scenes and exact native close/reopen

## IMPLEMENTATION

PR #62's real screenshot proved one Chrome sibling could be held visually invisible without moving the original NEWS-like viewport (0 new bright outside-mask points across 3470 samples). However its compositor used **one pointer** (`ci_sibling_actor`): a third native Chrome root could be disabled only once, not reasserted after later layouts or reclaimed safely after a close.

This narrow, opt-in CI-only change tracks up to 8 exact native Chrome sibling view pointers in a bounded static table. The compositor:
- ignores redundant observations of the same native view
- disables **each** tracked sibling's entire container scene after every layout transaction
- releases precisely the native view's slot on unmap, without freeing/reusing someone else's view
- disables even an overflow window's scene once but emits an explicit `SD_CI_CHROME_SIBLING_CAPACITY_EXCEEDED ... verification=FAIL` instead of falsely declaring capacity complete

The original cgroup/PID/starttime selection, early direct-floating workspace placement, fixed crop mask and default production build remain unchanged.

An additional `chrome-multi-toplevel` GitHub matrix job uses three exact 0600 stage-specific disposable request files. The owned Chrome service invokes the real `/opt/google/chrome/chrome --new-window about:blank` executable in the same isolated profile at stages 1, 2 and 3, with no network, browser sandbox bypass or operator workstation involvement. The runner requires:
1. Two **simultaneous** additional native XDG top-level roots, both kernel PID/cgroup-attested, and a real screenshot with zero extra sampled bright pixels and unchanged original XDG geometry
2. A **targeted native close** of one exact sibling ID. The other sibling remains mapped but invisible, and a second screenshot stays clean. The compositor must log a native scene slot revocation.
3. A third real Chrome invocation must create a **new native ID distinct from the closed ID**, while the survivor remains. A third screenshot stays clean, the compositor must have 3 quarantine events and at least one reclamation event, with no capacity overflow.
4. Exact Chrome service cgroup/process, native XDG, and private profile teardown must pass.

A green test proves only the captured **steady-state** frames and lifecycle behavior of this bounded 3-invocation experiment. It does **not** prove no early-frame flash, containment of all client-origin surfaces, or protected keyboard/pointer/focus routing.

## HOW TO USE

Review this PR's `Stage Director real Chrome stationary XDG moving mask (CI)`, matrix `chrome-multi-toplevel`. Look for `SD_CI_CHROME_MULTI_SIBLING_LIFECYCLE: result=PASS` and screenshot artifacts named `sd-ci-chrome-multi-two-open`, `sd-ci-chrome-multi-after-close`, and `sd-ci-chrome-multi-reopened`. None require local installation.

## VERIFY

- [ ] Source and production binary show CI-only bounded tracking, no production popup/browser policy change
- [ ] Real Chrome second/third/reopen requests confirmed from the same exact service in journal
- [ ] Two extra simultaneously mapped native XDG roots verified in Sway IPC and process generation
- [ ] All three captured real screenshots satisfy original window geometry and zero additional sampled bright escape
- [ ] Close targets exact sibling native XDG ID, surviving sibling remains mapped, a **new** ID appears on reopen
- [ ] Native slot revocation/reuse observed without capacity overflow
- [ ] Chrome mask/move, popup positive-negative, pending death, SIGKILL, production build, headless regressions remain green
- [ ] PR only merged after all current-head checks pass

## WATCH OUT FOR

Even with 8 slots, a ninth actor is only disabled once and **must** fail the verification; that is not an acceptable production capacity policy. Native window closure and event/focus dispatch remain separate security questions. Bounding this visual test does not attest Chrome's internal sandbox, authorize its popup scene, or allow Weather Station NEWS production browser launch. Current evidence samples 3,470 bright points outside fixed mask rather than every pixel at every frame. No live Hospital coordination connection or operator desktop acceptance is claimed.

## CHECKLIST

- [x] Check exact #62 merged master and Syntagma protocols
- [x] Replace one sibling pointer with fixed bounded slots and lifecycle cleanup
- [x] Add multi-stage exact Chrome real-executable requests
- [x] Add two-open, after-close and reopened independent captured-pixel checks
- [x] Retain production CI exclusion and existing user-machine no-write rule
- [ ] CI verification and merge
- [ ] Real early-frame / transient visual capture
- [ ] Seat, focus, pointer, keyboard, popup and subsurface confinement
- [ ] Actual attested Weather Station NEWS interior integration and operator acceptance

## NEXT

On green CI, plan a bounded **event/focus isolation** test for the hidden native sibling, and separately design frame-by-frame exposure measurements before any production browser authority. Use the current merged GitHub head and recheck ownership before implementation.

## DECISIONS

One small CI-only PR; no local Fedora, Weather QML, or production changes, no historical PR mass merging.
