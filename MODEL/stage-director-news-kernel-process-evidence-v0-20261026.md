# Stage Director SD1 // Linux process lineage evidence, candidate only

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`  
**STATE:** Unmerged source-only SD1 work stacked above NEWS shadow observation PR #8.  
**AUTHORITY:** Operator controls merge, live deployment and Hospital acceptance. No live Room/Assignment/worker registration assumed.

## IMPLEMENTATION

The Weather Station QtWebEngine viewer crashed Quickshell, so NEWS remains in safe mode. Stage Director's SD0 observer and SD1 prototype can count candidate Weather layer hosts and NEWS browser windows, but names and counts cannot prove ownership. This increment adds **kernel-backed identity and ancestry evidence** without granting any compositor power.

- `stage_director_process_evidence.h/.c`: a pure, independently testable policy. Given an expected root process PID, starttime (Linux `/proc/<pid>/stat` field 22), UID, mapped browser actor PID and observed starttime, follow a bounded parent chain. Deny wrong process-generation/UID, unrelated ancestry, loops, too-deep ancestry, vanished processes and reparenting/PID reuse during the check. Re-read every ancestor to detect identity changes.
- `stage_director_process_linux.c`: Linux-specific **read-only** `/proc/<pid>/stat` inspector. Parses field 22 after the final close parenthesis because Linux process names may contain spaces or `)`; obtains parent PID and a process UID from the procfs directory. Opens procfs entries with `O_NOFOLLOW`; does not launch any program or write to disk. Unreadable/non-dumpable process identity fails closed.
- C tests with simulated process trees cover same-process, direct child, grandchild, UID mismatch, cycles, process reuse, startup generations, missing processes, mid-check changes and max depth. Linux native tests read only the test runner's own process and validate tricky stat parser cases.
- `sway/meson.build` includes these modules. GitHub workflow runs pure C, Linux parser and static no-activation contracts.

**Scope limitation:** `LINEAGE_CANDIDATE` is **not** trusted ownership. The expected process root must eventually come from a compositor-owned, authorized launcher ticket minted for a specific Weather Station WATCH event, not a caller-supplied PID/starttime. The current code does **not** install that launcher, associate a real Wayland actor, wire new callbacks, start a browser, authorize clipping, or change focus. All states retain `owner_verified=false`, `can_bind_viewer=false`, `can_launch=false`, `can_clip=false`, `can_accept_input=false`.

## HOW TO USE

Code and tests only. GitHub CI compiles standalone tests. In a disposable checkout, `cc` can build `tests/stage_director/test_process_evidence.c` with `sway/stage_director_process_evidence.c`, and `tests/stage_director/test_process_linux.c` with `sway/stage_director_process_linux.c`.

Do not change or restart the operator's local SwayFX/Quickshell. The live Weather Station remains in safe mode.

## VERIFY

The relevant checks cover read-only kernel evidence, PID/starttime replay rejection, no duplicate owner claims and no links from the SD0 compositor observer, SD1 clip adapter or session code to this helper. Passing GitHub CI is not equivalent to a complete SwayPX/wlroots build or nested browser integration.

## WATCH OUT FOR

- **Kernel ancestry is not operator intent.** A process tree rooted at an arbitrary PID does not prove that a Weather Station action caused the browser launch.
- **Compositor Wayland PID:** SwayPX already obtains a `sway_view.pid` through client credentials, but its general workspace-launcher PID walk is not a Weather session security authority.
- **Browser multiprocess model:** Chromium may use descendants and intermediary helpers; process ancestry can change due to reparenting or sandboxing. The bounded traversal intentionally fails closed when lineage cannot be proven.
- **UID:** procfs directory ownership may become root for non-dumpable or privileged tasks; this is treated as non-match, not silently accepted.
- **Kernel race:** a read-only `/proc` traversal cannot eliminate all kernel process lifetime races. A future trusted launcher should hold a `pidfd` for its child, verify /proc starttime, and reject stale/redirected actor credentials.
- **No compositor activation:** do not set all clip authority booleans or reopen QtWebEngine based on `LINEAGE_CANDIDATE`.
- **Branch provenance:** SD0/SD1 work is stacked on development branches. SwayPX `master` does not contain this chain yet; pulling `master` does not merge draft PRs.

## CHECKLIST

- [x] Inspect current branch/PR state and existing launcher/view PID code
- [x] Add native read-only process lineage evidence
- [x] Test stat parsing, parent tree, UID, starttime and replay/teardown cases
- [x] Static no-actuation source guard and standalone CI
- [ ] GitHub CI
- [ ] Trusted compositor launch ticket and child pidfd ownership
- [ ] Verify actual Wayland client PID and app ID against trusted launch session
- [ ] Popup/input containment, post-layout clip restore and nested browser acceptance
- [ ] Operator approves merge/deployment

## NEXT

Implement **private operator-WATCH launch authority**, not merely a guessed PID. Trusted SwayPX session owner should mint a launch epoch for one mapped, verified Weather layer host, hold a child pidfd and process start marker, start one approved external browser in an isolated context, associate the actual mapped `sway_view` credentials to the session, and revoke all authority on CLOSE/unmap/crash. Only then should Stage Director consider enabling the compositor clip behind separately verified input/popup protection.

## DECISIONS

Prefer kernel-observed evidence over app ID or title guessing; preserve all denial defaults; no new live process, window movement, compositor mutation or local-machine write.
