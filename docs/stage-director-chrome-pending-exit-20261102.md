# Stage Director SD1 // Acknowledged pending request cancelled by browser death

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // Test native actor death with queued, not-yet-applied CI mask request
WORKER // External Hospital-compatible Doctor, not live Hospital registered
BASE // `master` `8c135ffa7278e06a052e3d82928e7b04d7f49058`
STATE // GitHub CI candidate, not operator-accepted playback

## IMPLEMENTATION

Extend PR #57's isolated Chrome screenshot regression with a **separate, deterministic test of a held viewport request**. Two independent GitHub Actions matrix jobs run on separate ephemeral runners: `after-third-mask` preserves PR #57's 3-mask SIGKILL recovery; `pending-before-move` holds an acknowledged request while Chrome's INITIAL 320x240 crop remains displayed.

In the pending scenario, only the opt-in CI scene-probe compositor recognizes the exact `PENDING\n` eight-byte marker in the existing securely opened 0600 request file. It revalidates exact native Chrome Wayland peer/cgroup and XDG geometry, logs `SD_CI_CHROME_MASK_PENDING_HELD`, reschedules its Wayland-thread request poller, and **does not change the clip or destination**. The runner captures a real PPM screenshot of the unchanged initial crop. It then SIGKILLs only its owned disposable Chrome service while the original file remains on disk. The existing native view-unmap lifecycle calls the CI scene-restore hook, which removes the poller and logs `SD_CI_CHROME_MASK_PENDING_REVOKED`.

The runner confirms exact process generations and systemd cgroup are gone, native Chrome XDG is absent from Sway IPC, and both restore markers appeared after the held request. The compositor must remain responsive while the request file stays present long enough to reveal an orphan timer or stale actor access. A second actual grim screenshot and Sway tree are compared to the pre-Chrome black background: both old and proposed masks and all four outside edges must have no browser pixels. The browser profile must also disappear.

## HOW TO USE

On the new draft PR, inspect `Stage Director real Chrome stationary XDG moving mask (CI)` and its two `real-chrome-four-edge-pixels` matrix jobs. For pending-abort use the `pending-before-move` job and the `sd-chrome-pending-*` artifacts. Nothing changes on the operator workstation.

## VERIFY

- [ ] CI source/syntax boundary, ordinary production build and opt-in compositor isolation.
- [ ] Existing post-PHASE3 SIGKILL and pixel proof still pass on its own runner.
- [ ] Pending marker observed by compositor and **no movement/rebind** before termination.
- [ ] Initial clip visibly held while pending; process/cgroup/XDG gone afterward.
- [ ] Native view unmap revokes timer; request file cannot act on a stale native actor.
- [ ] Real post-kill screenshot matches black pre-Chrome pixel baseline.
- [ ] All triggered GitHub checks and mergeability confirmed before landing.

## WATCH OUT FOR

**Scope precision:** This proves an acknowledged queued request was cancelled before scene mutation. It is NOT evidence of atomicity if a browser dies *inside* the C mutation statements, a transaction is half-applied, or a popup/multi-surface child survives. A real production viewer still lacks authenticated inner NEWS panel geometry, popup/focus/keyboard/pointer guards and an approved broker. `PENDING\n` is strictly for CI, never a production command. Historical failed PR #51 remains excluded.

## CHECKLIST

- [x] Reverify merged #57/current master and read Syntagma protocols
- [x] Reuse exact compositor timer, native scene restore and isolated Chrome service
- [x] Add CI-only acknowledged held-request branch and revocation log
- [x] Add real pending-before/after screenshot evidence checker
- [x] Run independent matrix CI variants with separate artifacts
- [ ] Verify CI and merge if checks succeed
- [ ] Test genuine mid-transaction browser loss and multi-surface popup/input confinement
- [ ] Coordinate authenticated inner Weather Station NEWS geometry with its owner
- [ ] Operator live testing and final acceptance

## NEXT

If both matrix CI jobs pass, land this small recovery proof. Next study whether the compositor has a safe scene-transition transaction boundary to test *actual* mid-operation interruption without introducing arbitrary delay or false confidence. Do not activate production browser playback on a test-only proof.

## DECISIONS

Preserve the default-deny production browser, no local Fedora writes or Quickshell QML edits; no mass merge of historical PRs. The operator retains final acceptance. No live Hospital registration is claimed.
