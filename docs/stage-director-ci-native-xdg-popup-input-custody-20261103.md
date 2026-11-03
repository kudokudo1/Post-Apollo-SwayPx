# Stage Director SD1 // Native XDG popup input custody (isolated CI)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE; no verified live registration
BASE // `master` `10367a41d6276f7bf5ed7c7bff82ea81e29cbb0a` (merged PR #67)
STATE // Candidate, GitHub checks required; production disabled
ASSIGNMENT // Prove an unrelated native XDG popup receives real input while an exact disposable browser-cgroup popup does not

## IMPLEMENTATION

Reuse the genuine XDG popup protocol fixture from PR #60 and real Wayland virtual pointer from PR #66. The unrelated control fixture now binds `wl_seat` and records real `wl_pointer.enter`, BTN_LEFT press and release, with `owner=POPUP` only for its own popup `wl_surface`. The harness selects a physical pointer coordinate from rendered magenta screenshot pixels, not guessed window geometry. Require client receipt AND compositor-side `target=OTHER` delivery at the positive control. When the same fixture requests a popup from the disposable Chrome service cgroup, retain the compositor's early root-scene denial, image evidence, then click the same position and require no service-side popup input receipt. No synthetic or fabricated client receipts qualify.

The only changed scope is the CI witness, Python harness, CI workflow and this recovery file. No `sway/desktop`, production browser launch, native compositor policy, Weather QML, Fedora or user desktop changes.

## HOW TO USE

Review this PR's `Stage Director real Chrome stationary XDG moving mask (CI)` > `native-popup-denial` matrix job. It should display both `SD_CI_NATIVE_POPUP_POSITIVE_INPUT: result=PASS` and `SD_CI_XDG_POPUP_INPUT_CUSTODY: result=PASS` only after the actual client-side receipts and compositor log checks. View `sd-ci-native-popup-control.log`, positive and denied images/artifacts for failure evidence.

## VERIFY

- [ ] Pinned native popup C client builds with `-Werror` on GitHub runner
- [ ] Client records ENTER + BTN_LEFT PRESS/RELEASE on POPUP; compositor logs actual unrelated-root input
- [ ] Same-service popup creation is denied and popup-colored pixels remain absent
- [ ] Click on the formerly painted popup coordinates produces no Chrome-service POPUP receipt
- [ ] All other Chrome CI variants and production-vs-test build boundaries pass
- [ ] Exact commit and PR green readback; local Fedora acceptance explicitly not attempted

## WATCH OUT FOR

This is a fixture-native `xdg_popup`, NOT one emitted by the Chrome executable. It is unrelated to native `wl_subsurface` parent ownership. The input-negative case checks absence of a denied popup receipt plus early native scene rejection, not production-capable routing or continuous transient frames. Screenshot comparison is sampled. The test intentionally does not allow any live WATCH browser launch.

## CHECKLIST

- [x] Inspect latest master, active PRs and existing popup / input fixture ownership
- [x] Add popup client pointer receipts and lost-capability handling
- [x] Choose input coordinates from actual rendered positive-control pixels
- [x] Reuse isolated virtual input sender; preserve positive/negative comparison
- [ ] Verify CI on exact PR head and repair within this test-only scope
- [ ] Merge only after proper authorization, green checks and exact head review
- [ ] Continue actual Chrome-created descendant and first-frame confinement testing
- [ ] Separate production Weather host/NEWS interior contract under operator-controlled integration

## NEXT

Read the `native-popup-denial` job first. If it fails, inspect exact log or client receipts before changing focus or input policy. After green, separately test Chrome-generated popup/child surface and early paint. Reacquire current GitHub and Room ownership before any new seam change.

## DECISIONS

Keep Stage Director browser launching OFF. No local installs, service restart or Quickshell modifications. No mass merge of historical PRs. No production client authority. Operator retains final acceptance.
