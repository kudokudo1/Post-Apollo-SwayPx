# Stage Director SD1 // Nonblocking NEWS supervisor and test-only child execution

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`  
**STATE:** Isolated feature branch above `stage-director/weather-news-launch-plan-v0-20261026`. No production launch activation.  
**AUTHORITY:** Operator has final authority. No live Hospital Room, Assignment, staff role or production activation presumed.

## IMPLEMENTATION

The long-term Weather Station goal is **NEWS → WATCH IN STATION → a browser contained inside the NEWS bay → × CLOSE**, without any QtWebEngine crash taking down Quickshell. Upstream SD1 implements geometry and a dormant wlroots clip API, NEWS source argv preparation, candidate browser process identity, and held pidfd custody. None of those provide trusted operator WATCH provenance or complete input/popup containment yet.

This bounded milestone introduces `include/sway/stage_director_news_supervisor.h` and `sway/stage_director_news_supervisor.c` for an **actual direct-child lifecycle** but does not launch any browser:

1. A supervisor records one child PID, kernel pidfd, process start-time generation, host reference and monotonically increasing epoch. It tracks EMPTY, RUNNING, STOP_REQUESTED, EXITED and LOST_REAPER. One active child per supervisor; no reusing prior epochs.
2. Stop uses `pidfd_send_signal(SIGTERM)` so it cannot accidentally signal an unrelated process following PID reuse. Poll uses **`waitpid(child, WNOHANG)`** for nonblocking direct-child reaping, never `waitpid(-1)` or a compositor-blocking indefinite wait. Finish closes only its own pidfd, clears process identifiers and preserves the epoch floor **after confirmed reaping**.
3. LOST_REAPER is fail-closed. If an external reaper consumes the child, this component will not silently claim success or permit a new child. The future live owner must integrate with Sway's SIGCHLD handling or a dedicated reaper before activation.
4. **Build-time safety gate:** the only spawn implementation is compiled under `SD_STAGE_DIRECTOR_TEST_HELPER`, runs a hardcoded harmless `/usr/bin/sleep 30` child through `posix_spawn`, immediately captures a pidfd and verifies UID/start-time, and destroys/reaps a child if verification fails. Production builds do not define this macro, do not expose that spawn symbol and have no browser-execution API. Neither source directly calls Sway's rendering, input, focus, or external browser plan.
5. Native CI creates the disposable test child, verifies CLOEXEC pidfd, refuses an incorrect epoch, uses pidfd signaling, polls to reap without blocking, closes the descriptor and demonstrates a second newer epoch. Other contract checks keep browser execution authority false.

This is real **process supervision under a test-only execution gate**. It is **not** a trusted browser launcher, and it does not prove an external browser is visually or interactively contained.

## HOW TO USE

There is nothing new to use on the operator's live system. GitHub CI independently builds the production helper to verify the spawn symbol is missing, then compiles the test-only variant with `-DSD_STAGE_DIRECTOR_TEST_HELPER` to execute the benign test child.

Weather Station's existing NEWS safe mode and `CHECK VIEWER` are unchanged. Do not install or restart SwayFX from this PR.

## VERIFY

- Standalone C test: test-only spawn of an absolute, fixed no-browser executable; fd CLOEXEC; nonblocking stop/reap; status; replay/duplicate/mismatched epoch refusal; descriptor cleanup.
- Build-time check: `nm -g` must find no test-spawn function in the ordinary production object.
- Static check: no caller from `stage_director.c`, the NEWS launch planner, existing pidfd custody, or the compositor clip adapter.
- Inherited SD1 geometry, actor/host identity, process-ancestry and no-actuation contracts remain in the same workflow.

CI does **not** verify complete compositor linking, browser profiles, safe runtime execution, nested Wayland behavior, popup/keyboard clipping, real NEWS video, systemd scopes, app-ID correctness on real Wayland, or a functioning WATCH/CLOSE action.

## WATCH OUT FOR

- A synchronous `waitpid` in the Sway event loop would freeze the desktop. This supervisor uses WNOHANG and expects the future owner to schedule bounded checks.
- A pidfd does not itself mean an operator authorized a WATCH request; even this test-only helper is not callable from the current Weather menu.
- `pidfd_send_signal` and `pidfd_open` require Linux kernel support; fail closed on unsupported systems.
- `posix_spawn` is an intentional test-only exception to the default no-launch rule. **Never** define `SD_STAGE_DIRECTOR_TEST_HELPER` in production Meson or expose that flag through user-facing configuration.
- Only `/usr/bin/sleep` is executable in this test. No arbitrary URL, executable, browser, command, shell or network access is accepted.
- The supervisor does not force SIGKILL on a slow browser or implement profile cleanup. A future live controller needs a bounded shutdown escalation and verified child-only cleanup, with process group/descendant behavior accounted for.
- SD0/SD1 has been merged along development branches but not activated on SwayPX `master`; a taskbar pull of master is not a deployment.
- No current source authenticates a Quickshell WATCH click, mapped layer surface, private compositor owner and browser child in one trust chain. Don't bypass that using Booleans, names or matching count.

## CHECKLIST

- [x] Reinspect Stage Director development HEAD, Syntagma rules and current PR status
- [x] Implement one-process nonblocking stop/reap/finalize lifecycle
- [x] Gate all process creation behind test-only build flag
- [x] Require test-only start to verify exact child process generation by pidfd
- [x] Add ephemeral-child tests and production symbol absence check
- [x] Preserve runtime compositor authority = NONE
- [ ] GitHub CI green
- [ ] Full SwayPX isolated build/ABI test
- [ ] Authenticated WATCH intent, host/session mapping and trusted browser executable
- [ ] Verified private profile directory, real child ownership, daemon/SIGCHLD integration
- [ ] Actual compositor popups/input/clip/teardown acceptance in nested Wayland
- [ ] Operator-approved merge and separate deployment decision

## NEXT

Implement a real **trusted WATCH-to-session owner** that binds a specific mapped Weather Station layer-shell surface and its NEWS bay to a private operator intent, without allowing clients to fabricate its authority. Before introducing browser process execution, the team must demonstrate a clip enforced on actual content *and* pointer/popup/keyboard behavior in a nested compositor, and connect the session teardown to a verified private reaper. Only then replace the test-only fixed helper with tightly checked browser execution and registered source IDs.

## DECISIONS

Prefer a genuine supervised-child lifecycle test rather than writing an unreviewed live process launcher directly into SwayFX. The only process launched in CI is a disposable benign helper; production has no spawn entrypoint. Nothing is merged, activated or changed locally by this PR.
