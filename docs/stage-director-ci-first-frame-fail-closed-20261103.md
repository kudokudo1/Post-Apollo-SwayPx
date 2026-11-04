# Stage Director SD1 // CI-only first-frame fail-closed Chrome scene

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // Verify native Chrome root is hidden at map until its first source-verified crop is prepared at compositor output build
STAFF ROLE // DOC, external Hospital-compatible; no verified live registration
BASE // `master` `1508d84b7d4f60b4e96af7278416f4f5e0605112` (PR #68 merged)
STATE // CI draft candidate, no production permission

## IMPLEMENTATION

In the existing separately compiled `SD_STAGE_DIRECTOR_CI_SCENE_PROBE` binary, an explicit `STAGE_DIRECTOR_CI_CHROME_FIRST_FRAME=1` flag hides the exact, kernel-cgroup-matched Chrome container scene at `view_map`, before normal workspace layout. It does not use browser title, app ID or request metadata as identity.

The real output repaint callback calls the CI-only pre-frame guard **after** regular `output_configure_scene` and **before** `wlr_scene_output_build_state`. If there is no selected source-authenticated Chrome actor with a valid, unchanged geometry and crop, the entire actor scene stays hidden. Once ready, it reapplies both the source clip and normalized buffer destinations and only then enables the scene. It rechecks custody on each output build. The guard reference is revoked at native `view_unmap`.

The new `chrome-first-frame` CI matrix lane requires `SD_CI_FIRST_FRAME_MAP_HIDDEN` to precede `SD_CI_FIRST_FRAME_RELEASE`, absence of custody-loss errors and actual later Chrome four-edge screenshot verification. Existing nine Chrome modes remain separate, unmodified and mandatory.

## HOW TO USE

GitHub Actions > `Stage Director real Chrome stationary XDG moving mask (CI)` > `real-chrome-four-edge-pixels (chrome-first-frame)`. Require `SD_CI_FIRST_FRAME_NATIVE_GUARD: result=PASS` together with pixel checker, exact Chrome process provenance, stop and cleanup. Review all workflow jobs, not just the new lane.

## VERIFY

- [ ] Distinct production compositor compiles without any first-frame CI gate
- [ ] CI compositor compiles source and output hooks without warnings
- [ ] On actual Chrome native map, selected cgroup-matched view is hidden before ordinary layout
- [ ] Before its first eligible output build, crop and destination are restored and only then scene enabled
- [ ] No custody-loss marker and actual post-map screenshots still obey four edges
- [ ] All Chrome regressions green and exact PR head read back
- [ ] Operator Fedora and Weather Station acceptance NOT attempted

## VERIFICATION HISTORY // CHROME STARTUP PRESSURE

Initial PR #69 run: `chrome-first-frame` passed native map-hide, output release and four-edge screenshot checks. The first full Chrome matrix reported `BLOCKED_NO_UNIQUE_LIVE_NATIVE_CHROME_TOPLEVEL` in `chrome-multi-toplevel` and `chrome-subsurface-owner` before either lane reached its feature checks. The multi-window lane passed on isolated retry; subsurface startup timed out again. This is a startup-provenance timing blocker, not a rendered-pixel containment failure. The CI harness now gives these two multi-client modes a bounded 160 observation samples (formerly 80), preserving native kernel PID/starttime, exact transient cgroup, unique XDG window and three stable geometry observations. Rerun all checks on the changed PR head before merge.

## WATCH OUT FOR

This experiment provides **CI-only scene authority gating at output preparation**, not a guarantee that all pixels emitted by the GPU were continuously observed. The screenshot checker still samples frames, and the Chrome app is fixed `about:blank`. No live Weather host interior geometry, ephemeral popup mediation, sandbox attestation, browser launch broker or actual WATCH playback is enabled.

The new gate is deliberately opt-in under a separately compiled CI binary. On failure it hides Chrome; it must not change normal SwayPX rendering. Any build, cache, native-provenance or pixel failure blocks the PR and must not be explained away as a PASS.

## CHECKLIST

- [x] Read Syntagma and Hospital-compatible scope
- [x] Verify current `master`, latest PR #68, historical open PRs and ownership
- [x] Find native map, view unmap, output layout and output state build seams
- [x] Add isolated map-hide and pre-frame crop gate without production activation
- [x] Add dedicated output, harness and workflow boundary checks
- [ ] Green CI on exact head and repair within assigned test-only seam
- [ ] Merge only after verified allowed workflow; preserve operator final acceptance
- [ ] Next: actual browser-created popup/descendant behavior and authenticated Weather interior contract

## NEXT

Inspect `chrome-first-frame` CI logs for first native crop authorization and actual screenshots, then ensure no existing matrix tests regress. Continue Chrome-created descendants in a separate source-controlled test, not by silently enabling browser playback.

## DECISIONS

No local Fedora modification, no Quickshell QML edits, no production browser launch, no bulk-merging old experimental PRs, no live Hospital registration claims. The operator retains final acceptance.
