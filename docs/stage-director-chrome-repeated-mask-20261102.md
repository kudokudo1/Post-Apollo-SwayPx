# Stage Director SD1 // Two successive real Chrome mask movements

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // CI-only repeated native Chrome mask movement and screenshot integrity
BASE // `master` at `6e1e124dc115a14fd183c7778c0abc9b618e3c12`
STATE // Implementation candidate; CI pending; not an operator-accepted viewer
CONNECTION // External Hospital-compatible ChatGPT doctor; live Hospital registration unverified

## IMPLEMENTATION

Extend the prior one-shot, isolated real Chrome SceneFX crop proof to **three rendered states within one actual disposable headless SwayPX compositor**, with two successive mask moves and screenshots at every state:

| State | Chrome native XDG outer rectangle | Visible mask |
|---|---|---|
| Initial | Stationary 1920x1080 | (96,96) 320x240 |
| PHASE2 | Unchanged native XDG PID/window ID and rectangle | (520,240) 400x300 |
| PHASE3 | Still unchanged native XDG PID/window ID and rectangle | (1040,560) 280x220 |

The compositor's CI-only Wayland-thread timer rechecks exact native Chrome peer/cgroup identity and expected full-window geometry before each request. Both PHASE2 and PHASE3 are serialized via the same exact-name, single-owner 0600 test file; stale PHASE2 is held harmless while its screenshot is obtained, and PHASE3 must be an exact seven-byte marker. Missing/mismatching actor, invalid marker or bounds restores the default scene and fails closed.

The new screenshot checker requires actual bright pixels in the expected mask at each phase, black pixels at BOTH old locations after PHASE3, clear black baseline at the new location before PHASE3, outside-background stability, and four-edge exclusion. It checks the same native XDG ID, PID, border and outer 1920x1080 rectangle across all phases. Actual PPM/PNG screenshots and Sway trees are retained as CI artifacts.

## HOW TO USE

Review the GitHub PR's `Stage Director real Chrome stationary XDG moving mask (CI)` workflow, screenshot/artifact and test logs. No UI route exists because this experiment must never be activated on the operator's Fedora desktop.

## VERIFY

Require both CI jobs:

- Source boundary: no test-only scene selector in the normal binary, no IPC/QML/browser authority, source and screenshot checker syntax and expected marker assertions.
- Disposable real Chrome proof: full normal SwayPX build, separate opt-in SceneFX test binary, real Chrome headless runtime, three actual screenshot states and process/cgroup teardown.

The proof is not complete until the GitHub CI real-Chrome job passes. A scene-tree hit-test alone is insufficient.

## WATCH OUT FOR

This demonstrates **repeated** rendering behavior only for one exact CI Chrome identity, fixed windows and three fixed masks. It still does not implement an authenticated production NEWS inner viewport controller, popup and multi-surface confinement, pointer/keyboard/focus routing or sandbox attestation. The prior failed outer Chrome window motion PR #51 remains excluded. No Weather QML owned by another worker is touched.

## CHECKLIST

- [x] Recheck `master`, active historical PRs and previous CI-only mask proof
- [x] Implement serialized PHASE2 and PHASE3 scene mask transactions with identity rechecks
- [x] Require separate PHASE3 PPM/PNG and native window/tree evidence
- [x] Add triple-screenshot stale-pixel/edge checking
- [ ] Verify GitHub source boundary and real Chrome headless mask proof
- [ ] Merge only after all required checks and mergeability preflight
- [ ] Implement actual NEWS inner native surface provenance with Weather Station owner
- [ ] Add popup/seat confinement and production broker before any activation

## NEXT

After green tests, continue with abnormal Chrome exits during/between mask transitions and cleanup/revocation proof. This must include pixel evidence, not only process exit status, before active viewer work.

## DECISIONS

Continue clean master-based PRs; preserve old #23–52 as history rather than mass-merge. Do not edit Weather Station QML, move actual Chrome outer XDG window, or enable production launch/input/popup/clip. No local Fedora write or live Hospital registration is claimed.
