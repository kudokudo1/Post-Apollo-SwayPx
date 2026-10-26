# Stage Director SD1 // Weather NEWS held process identity

**Status:** Unmerged, isolated development milestone stacked above the NEWS process-evidence work (PR #9).  
**Patient:** `kudokudo1/Post-Apollo-SwayPx`.  
**Authority:** Operator. No Hospital Room, Assignment, live staff role or deployment is presumed.

## IMPLEMENTATION

The Weather Station NEWS safe-mode viewer needs a browser in an independent process, eventually clipped into a verified Stage Director viewport. The prior code can inspect candidate host/actor windows and validate Linux process ancestry, but a PID may be reused and a matching app ID is not process ownership.

This milestone adds **real kernel pidfd custody**, without creating or taking control of browser processes:

- `include/sway/stage_director_news_custody.h` declares one candidate process holder with an epoch, mapped host reference, exact PID/starttime/UID record, and one owned CLOEXEC pidfd. The epoch floor survives CLOSE and rejects replay. This is private C API, not IPC.
- `sway/stage_director_news_custody.c` reserves a single host candidate and captures a pidfd using Linux `pidfd_open` only after validating the expected process generation. It then re-reads /proc to reject a changed PID/starttime/parent/UID and checks that the process has not exited. `poll(pidfd)` reports child liveness without signals. Release only closes Stage Director's own descriptor, clears the root identity and retains epoch floor. Failure to open pidfd or read process identity is a hard refusal.
- Native tests create **only one harmless temporary test child** in an isolated GitHub runner, check wrong UID/starttime/epoch, descriptor CLOEXEC, liveness, process exit, CLOSE cleanup, and replay protection. No browser, IPC, network or compositor is launched.
- Build and source-boundary tests show the custody helper is included in Meson but disconnected from all SD0/SD1 view, layer, session and clipping events.

Every returned result remains `owner_verified=false`, `can_launch=false`, `can_bind_viewer=false`, `can_clip=false`, `can_accept_input=false`. The held pidfd prevents accidental PID-generation substitution **only when the expected root itself is supplied by a future authenticated launcher**. The current code does not mint that root.

## HOW TO USE

This is source-only. CI runs a C11 native Linux test; there is no Weather Station button or supported user operation yet. Keep the existing NEWS WATCH crash-safe screen. Do not run or replace the operator's SwayFX/Quickshell.

## VERIFY

The GitHub workflow tests pidfd capture of a controlled child, process-exit detection, CLOEXEC, zero-runtime-authority contract and all inherited SD1 policy checks. This is **not** a full SwayPX linker/build or nested Wayland rendering test. It does not prove process ownership relative to an operator WATCH request.

## WATCH OUT FOR

- A pidfd is a kernel-stable process handle, **not** proof of origin. Taking a pidfd for an arbitrary user-supplied PID would be an ownership vulnerability. Do not expose the capture function to generic IPC/QML or call it merely because a browser name matches.
- Closing a pidfd does **not** terminate a browser. A future supervised launcher must own one process, decide controlled termination, prevent orphan windows and handle child reaping.
- The current session code tracks a provisional launch epoch; there is still no authenticated handshake between a specific Quickshell WATCH action, its mapped layer surface and a browser child.
- Even correct process custody cannot activate clipping until the complete rendering, popup, pointer/keyboard and teardown gates are actually installed and tested.
- Linux pidfds depend on kernel support and permissions. Unsupported or unreadable process evidence stays denied.
- SwayPX `master` does not contain the entire SD0/SD1 development chain. A taskbar pull of `master` won't deploy these drafts.

## CHECKLIST

- [x] Inspect PR #8/#9 state and upstream source
- [x] Add a one-candidate pidfd custody component
- [x] Verify kernel starttime+UID around pidfd capture
- [x] Add epoch, close, process-exit and CLOEXEC regression tests
- [x] Register in Meson and CI, prevent implicit activation
- [ ] CI green
- [ ] Trusted WATCH-request-to-launcher contract with a private, allowlisted browser execution path
- [ ] Process ownership/child reaping and view association with native Wayland credentials
- [ ] Full SwayPX build and nested compositor acceptance
- [ ] Input/popup/clip gating, rollback and operator-approved merge/deployment

## NEXT

Implement a compositor-owned private launch controller that receives an authenticated WATCH request, verifies one mapped Weather host, chooses only a known browser executable and registered source target, starts the child with a separate profile, captures its pidfd and process generation, and ties a single compositor-observed view to the active lease. Never launch a browser until the nested compositor has real paint/input/popup containment and CLOSE recovery. Stage Director's existing candidate-only modules remain the fallback.

## DECISIONS

Introduce kernel process custody as one separate, reversible component; do not bypass the missing trusted launcher, treat unique candidates as ownership, or enable production browser playback early.
