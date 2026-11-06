# Stage Director M22 // Company MapLibre process parentage checkpoint

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre containment
STAFF ROLE // Nurse 2 (operator-authorized external continuation)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, NOT REGISTERED
ASSIGNMENT // Verify native Company host-to-Qt actor Linux process parentage with actual M20 Wayland observations
BASE // master c027b86798a7376c56315e080b8e800c71d3f021
PHASE // Bounded GitHub M22 implementation, not live desktop installation

## IMPLEMENTATION

The operator granted broad implementation ownership of Company Stage Director integration. The known, tested M19 #74, M20 #75, M21 #76 stack was merged in order using merge commits, resulting in SwayPX master c027b867.... No local Quickshell/compositor changes were performed.

Current Company launch mechanism inspected in The-Post-Apollo-Project:
- CompanyCoveragePane user explicitly chooses OFFLINE TEST or separately consented ONLINE STREETS.
- Quickshell's CompanyVectorSidecarController.qml uses a single attached Process to start /usr/bin/python3 CompanyVectorSidecarLaunch.py.
- The launcher uses os.execv() to replace itself with the owner-local M15 company-vector-probe bridge; the bridge uses os.execve() to replace itself with the native C++ Qt map process. This preserves the PID/start-time of the original Quickshell child.
- The Qt probe's application name is a diagnostic hint only, not actor trust.

M22 adds `stage_director_company_lineage.h/.c` to SwayPX:
- Requires exact native Wayland host/actor candidates already observed by M20 with distinct Wayland client refs, matching nonroot UID and live stable kernel process identities.
- Re-reads Linux /proc for the host/actor and requires the native Qt actor to be the **direct child** of the mapped Company/Quickshell Wayland client process. Unlike a generic ancestry match, a grandchild launched through a separate terminal/controller does not count.
- Reuses existing `stage_director_process_check()` to reject PID reuse, reparenting, UID divergence and unstable ancestor walks; re-reads host and actor again after that pass to guard against exit/reparent races.
- Returns a strictly non-authorizing `CANDIDATE_ONLY` parentage result. No implicit Company actor custody or M19 compositor_selected lease exists here.
- Production `stage_director.c` on a Qt app-name diagnostic event scans its own live layer registry, insists on **exactly one** currently native-observed Company layer, then runs M22's /proc-backed direct-child probe and emits a debug-only status. Ambiguous/missing hosts give no evidence. No registration, permission, frame, input or scene mutation.
- Adds pure-C tests for direct child, unrelated Qt app, same UID spoof, grandchild, PID/UID/start changes, missing /proc, unstable reparent, invalid or XWayland-like native peer candidates. Full pinned-Nix SwayPX build remains required.

This is a real OS-level evidence improvement, not actual authenticated operator consent. Other unrelated Quickshell child processes may have the same kernel relationship. A separate secure launch-intent binding must be established before issuing the M19 compositor-selected lease or rendering.

## HOW TO USE

There is **no new UI action**. The existing Company → Coverage → VECTOR VIEW still launches a separate Qt window. M22's debug-only `[stage-director][COMPANY_M22][PARENTAGE]` line will become observable only after a separate, specifically approved local installation of a SwayPX build containing M22, and only if the Qt application exposes the expected app-id hint. The log is not authorization to move windows.

## VERIFY

- Syntagma AGENTS, Hospital Agent and Authority protocols consulted.
- Exact M19/M20/M21 GitHub PR checks verified green and merged in chronological dependency order.
- Current post-merge SwayPX master and the exact Company Python exec chain inspected.
- CI tests source checks, native process semantics and complete SwayPX compilation. Follow exact PR head check runs, not stale previous summary.
- No evidence of real live direct-child relationship, Quickshell output app-id, actor clipping, geometry wire, or input handling yet.

## WATCH OUT FOR

- Even proven direct parentage cannot distinguish malicious/unrelated Quickshell children from a deliberate Company launch. **It is only candidate evidence** and must remain insufficient for SwayPX scene/input privileges.
- The actual Qt root may be reparented after Quickshell termination. Parentage must then fail closed. Multiple Company namespace layers must also fail closed; no arbitrary first match.
- The app ID/namespace are not authentication; the diagnostic hint is never part of the pure lineage policy.
- No local desktop writes, package changes, Quickshell restart, compositor replacement, system SwayPX activation or copy from GitHub were performed.
- Other Weather and historical Chrome Stage Director work stays outside scope.

## CHECKLIST

- [x] GitHub state/ownership/protocols rechecked
- [x] User-authorized merge of fully green M19-M21 foundations to master
- [x] Model the verified process-preserving Python exec chain
- [x] Native Linux host→Qt direct-child, stable process preflight
- [x] Map-time read-only single-Company-layer diagnostic
- [x] Pure unit tests, legacy lineage regression, full-build CI
- [ ] Exact-head M22 CI full compositor and tests passing
- [ ] M22 PR review and merge if green and scope safe
- [ ] Authenticated Company launch-intent and M15 process birth receipt
- [ ] Private same-QPA Company Wayland geometry report and compositor epoch
- [ ] Scene crop/size, pointer/keyboard, focus, popup and siblings
- [ ] User-local controlled rollout and operator final acceptance

## NEXT

After M22 green, land this inert audit addition under the operator's implementation delegation. The next milestone should carry **trusted launch-intent custody** from an explicit Company offline/online click to the exact M15 Qt child process. It cannot be inferred from process ID or parentage alone. This likely needs a private Company-specific Wayland host request and an independently observed child birth evidence channel; the existing Weather WATCH protocol should remain isolated. Only then attempt scene clipping in headless compositor tests.

## DECISIONS

Full implementation responsibility applies to Company Stage Director and its reviewable GitHub integration. No unbounded ownership of SwayPX/Weather, live Hospital registration, local machine overwrite, or bypass of production acceptance. Existing M15/M17 MapLibre user installation remains the working baseline.
