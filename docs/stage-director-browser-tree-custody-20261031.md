# Stage Director SD1 // Browser-like process tree custody boundary

STATE // Draft, unmerged, no live browser. Native Linux process-group teardown and CI-only disposable two-process fixture.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director Weather Station NEWS viewer integration.

ASSIGNMENT // Operator-approved Option A NEWS-first compositor integration, browser supervision and cleanup milestone.

WORKER // OPEN AI // CHATGPT, external Hospital-compatible doctor; no live registration verified.

BASE // [PR #45](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/45) at `00e6fe03f4a244a6fca160c317050d8ecce0af1f`.

NEIGHBOR // Separate Weather Station worker owns Weather QML and UI; none modified.

## IMPLEMENTATION

Source inspection found that `stage_director_news_supervisor_request_stop()` held a `pidfd` and sent SIGTERM to one direct child only. A Chromium-like browser spawns additional processes; stopping only its root may leave children alive. This milestone adds **owned process-group metadata** to the existing supervisor, not a second process manager.

The default legacy single-child path still uses `pidfd_send_signal` unchanged. A future directly spawned process group may instead use a group stop **only if** the tracked group ID equals the unreaped child PID, the held `pidfd` says the root is alive, Linux /proc reports the exact original starttime and UID, and `getpgid(child_pid)` confirms the child remains its own group leader. The current direct parent remains unreaped until after group signal, eliminating a numeric group ID reuse window at signal time. `finish()` refuses to mark a tracked group complete unless a group stop was issued. This still does **not** prove every descendant has stopped; real browser/cgroup custody must be separately verified.

The ONLY new group launcher is build-gated behind `SD_STAGE_DIRECTOR_TEST_TREE`, not defined in ordinary SwayPX. It `posix_spawn`s a fixed, compiled C fixture in a disposable GitHub runner with a new process group. That fixture forks exactly one descendant and shares a private readiness marker. The test runs as a Linux child subreaper, verifies native PID/UID/starttime and both process-group memberships, denies stale epoch and foreign stop attempts, sends a verified SIGTERM to the owned group, waits for the root, and reaps the descendant. No shell, URL, browser, network, QML, Wayland command, arbitrary PID or scene/input mutation is part of this fixture.

## HOW TO USE

Review the draft PR and `Stage Director browser-like process tree custody (CI only)` workflow. No operator computer action or local installation is requested. The process-tree fixture only runs in a disposable GitHub runner, after explicit CI environment guards. There is no production browser launch API.

## VERIFY

Run both legacy `test_news_supervisor.c` and new `test_news_supervisor_tree.c` against the same existing supervisor source. The disposable actor actually forks a descendant, and the parent test confirms both terminate after the one authenticated group signal and are reaped. Separately compile ordinary production SwayPX via pinned Nix and verify the CI-only fixture marker is not linked. GitHub CI pending at PR opening.

## WATCH OUT FOR

- **Process groups are not complete browser sandboxes.** Descendants can fork, `setsid()`, change groups, or survive as independently managed processes. The production viewer must use a proper cgroup/session ownership design and verify all children have exited before declaring cleanup complete. The test intentionally does not claim this.
- A root whose identity or pgid no longer matches is **denied**, not signaled by guessed PID. A group kill failure must remain an observable blocker, never silently report success.
- `kill(-pgid, SIGTERM)` is a group-wide actuation. That path is dormant in production until an independently verified, compositor-owned browser spawn supplies and owns that exact group.
- No Chromium executable is run in this milestone. Do not interpret `browser-like` as browser playback, active renderer or browser sandbox verification.
- The underlying supervisor still cannot launch production browsers; the existing fixed source launch plan remains non-executable. WATCH currently reserves only a native candidate lease, and all scene/input/focus/popup authority flags are false.
- PR #43's private-global discovery leak fix remains a required ancestor of this stacked work. Do not deploy PR #42 alone.
- The runner test deliberately uses a Linux child-subreaper to reap the forked descendant. SwayPX does not itself become a subreaper.

## CHECKLIST

- [x] Reinspect current PR #45, Syntagma, supervisor and existing process custody
- [x] Reuse existing supervisor; add owned process-group verification and stop
- [x] Preserve original single-child pidfd stop path
- [x] Keep the only new process launcher CI-gated, fixed-path and absent from production
- [x] Add a real forked descendant fixture and subreaper test with exact group-stop/reap checks
- [x] Add source authority guards, legacy supervisor test, normal production Nix build
- [ ] Verify GitHub CI and repair any faults
- [ ] Verify actual Chromium/Brave multi-process process tree under isolated cgroup
- [ ] Add a production-authorized broker with private browser profile and clean lifecycle
- [ ] Verify wl_client native browser lineage, pixel clip, popup/focus/input containment
- [ ] Integrate real Quickshell Weather Station bridge through separate owner
- [ ] Operator-approved desktop acceptance and rollback

## NEXT

After CI green, move to a disposable **actual browser** process under a private cgroup/profile and inspect all descendant/cgroup exit paths. Only then consider exposing a production broker, and do not permit it to be invoked by WATCH until actor identity, clip/input/popup/teardown guards are proven.

## DECISIONS

No merge, local machine mutation, Weather QML change, browser launch, scene/input authority or live Hospital registration was performed. Production group signal code is enabled only for an owned group that has no production issuer yet; final operator acceptance remains outstanding.
