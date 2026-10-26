# Stage Director // Weather NEWS viewer session candidate V0

**Patient:** `kudokudo1/Post-Apollo-SwayPx`  
**State:** Isolated source implementation, no live activation  
**Dependency:** SD0 scene observer → SD1 V0 viewport planner (PR #4) → SD1 V1 dormant wlroots clip (PR #5) → THIS candidate session lifecycle.  
**Operator:** final authority. No Hospital Room, Assignment, worker ownership transfer or session registration assumed.

## IMPLEMENTATION

The Weather Station NEWS module crashed Quickshell when its embedded `QtWebEngine` constructor aborted. Its browser viewing path is currently disabled for safety. A separate browser process and proper Stage Director scene containment are needed, but *external browser identification is not proof of process ownership*.

The new `stage_director_viewer_session.h/.c` introduces **one candidate viewer at a time**. It deliberately does not enable the existing compositor clip or start a program:

1. **Reserve:** store one host reference and observed host/output/NEWS viewport snapshot for a **strictly newer**, nonzero launch epoch; accept only a positive expected PID and nonzero process-start marker. Reservations fail for duplicate sessions, invalid rectangles or a bay escaping its host/output.
2. **Attach:** correlate one browser actor reference with the exact host/epoch/PID/process-start pair. Reject stale epoch, foreign host, forged/same-handle actor, process mismatch and actor bounds not fully covering the viewport. An app ID is only an identification hint from the SD0 profile.
3. **Probe:** compare actual current geometry against the original host/output/viewport/actor snapshot and call SD1's viewport validity policy. A successful probe means **candidate matched**, *never* `can_launch`, `can_clip` or `can_accept_input`.
4. **Invalidate:** explicit CLOSE, host unmap/destroy or actor crash/unmap releases the candidate, clears the handles and PID, but **retains the greatest used epoch** to reject replay while this compositor-side session instance survives. A new authorized launch must have a newer epoch.
5. **No side effects:** no dynamic allocation, shell calls, IPC, browser launch, wlroots node mutation, keyboard/pointer handling or QtWebEngine. The module compiles independently as C11 and is registered in SwayPX Meson but disconnected from runtime entrypoints.

**Trust distinction:** The epoch, PID and start marker are *values from a future trusted compositor-side launcher*, not authentication if copied from user-controlled JSON or guessed. This candidate code alone cannot assert that an OS process is owned by Weather Station. It also does not secure the process-start marker against arbitrary spoofing without future launcher provenance checks.

## HOW TO USE

Source checks from the PR branch, without a compositor:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  sway/stage_director_viewport.c \
  sway/stage_director_viewer_session.c \
  tests/stage_director/test_viewer_session.c \
  -o /tmp/sd-viewer-session-test
/tmp/sd-viewer-session-test
python3 tests/stage_director/test_viewer_session_boundary.py
```

These commands do not launch browser content, mutate Sway, or modify Quickshell. Official CI runs the same checks. The Weather Station's WATCH safe-mode interface and existing separate-process `CHECK VIEWER` remain unchanged.

## VERIFY

Native C tests cover: successful reservation/attach/probe; strict process start and PID matching; exact host and actor references; concurrent launch denial; same-epoch and stale-epoch replay; close and re-open; host unmap and browser exit teardown; invalid/overflowed viewport; resize/output change; mapped/output/identity disappearance; and perpetual non-authority. Python static guard checks that no SD0 or clip runtime callsite exists and that no scene, input or process mutations occur.

**Not verified:** complete SwayPX link/build; the trusted compositor launcher and its process ancestry; a real mapping to a specific Weather layer-shell host; independent browser paint/input clipping; browser popup containment; nested compositor video; or operator's local runtime. These require separate stages.

## WATCH OUT FOR

- Source-only handle identity (uintptr_t), PID and start markers prevent simple accidental collisions but are **not** an authentication protocol. Never expose `session_reserve` or `session_attach` as raw user-controlled IPC. Trusted compositor ownership must mint tickets and hold actual native host/actor references.
- Existing SD1 V1 clip needs true input hit-test, popup and teardown guards. Do not force its authority structure to all-true merely because a candidate matches.
- Invalidation must be driven by verified host/actor lifecycle hooks in a later branch; a browser closing and reopening should never inherit a previous epoch.
- Geometry is compositor-global *logical* coordinates; do not hardcode display names, 1000×700 Station location or an old terminal crop.
- Do not merge the stacked PR directly into SwayPX `master`, bypassing SD0 and PR #4/#5. Their separate ownership and integration state must be reconciled first.
- Tests do not imply a production-safe external browser integration.

## CHECKLIST

- [x] Read current SwayPX branch/PR status and SD1 V1 adapter
- [x] Add single-candidate session lifecycle
- [x] Implement epoch replay and process identity matching
- [x] Deny geometry drift and clean up on close/unmap/crash
- [x] Register module and add C + static CI checks
- [ ] CI green
- [ ] Full compositor build in an isolated environment
- [ ] Trusted process-owned host/actor binding and lifecycle hooks
- [ ] Real input/popup guards and post-layout clip reapply
- [ ] Nested real browser test with close/reopen/failure handling
- [ ] Operator merge/deployment authorization

## NEXT

**Trusted session owner milestone:** implement a private compositor-side launch/association layer tied to the active Weather Station layer-shell surface, an authenticated operator WATCH intent, and an independently verified browser child process. Its role is to supply real observations to this lifecycle code. Do not authorize clipping until the popup, input, teardown, and post-layout restoration checks are actually implemented and tested under nested SwayPX. Pair with Weather NEWS transport only after those gates.

## DECISIONS

Maintain a source-only, single-viewer, fail-closed candidate state while SD0 and SD1 prerequisites remain unmerged. Keep process/session association separate from actual compositor clipping and from live user activation. No local machine changes or merges.
