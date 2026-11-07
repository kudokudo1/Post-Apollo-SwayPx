# Stage Director // Social W real Wayland two-window baseline

**STATUS // CI-only native Wayland runtime experiment, NOT a compositor group fix.**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`; Social W owner is `kudokudo1/The-Post-Apollo-Project`.  
**DEPENDENCY //** Stage Director Social group geometry draft PR #94. External Vesktop-focus lifecycle PR #1427 is merged; operator visual acceptance is pending.  
**OWNERSHIP //** No shared compositor, Company M30, Weather or original CI harness modifications. This continues the external Hospital-compatible Social W assignment, without assuming live Hospital registration or a transferred Stage Director owner.

## IMPLEMENTATION
A new isolated GitHub-only harness builds the **ordinary SwayPX compositor** from the repository, launches real native Wayland `foot` XDG toplevels in a disposable 1920×1080 headless output, and identifies exact Sway container IDs for Social chassis, guest and unrelated control. Config/environment are private under GitHub `RUNNER_TEMP`, Nix store dependencies are pinned, and all processes/sockets are torn down.

The script positions both real windows using the existing Social selector=110/inset=1 rectangle relationship, validates host-led and guest-led move/resize, checks unrelated identity boundaries, and captures **actual grim screenshot pixels** from settled host, guest, unrelated and background regions. It then kills just the CI guest process, requires the chassis to retain geometry, launches a replacement guest with a different Sway ID and docks that one.

An explicit intermediary witness moves the chassis **without moving the guest**, and confirms Sway sees the resulting split. This demonstrates the structural failure mode of two independently commanded windows. It is **not a timestamped drag-latency measurement**, and does not prove a scene-frame grouping repair.

## HOW TO USE
GitHub draft PR CI only. The test is guarded by `GITHUB_ACTIONS=true`, `CI=true`, a private exact GitHub temp path, a dedicated flag and Nix store binaries; no local Fedora desktop, Sway config, Social QML, browser account, login or system state is touched.

## VERIFY
Require the dedicated workflow to build real SwayPX, map three genuine Wayland clients, pass exact Sway IPC geometry and actual compositor pixel samples, close/reopen the guest, and cleanly exit. No source-static substitute may count as rendering evidence. Previous pure grouped geometry PR #94 tests only calculations.

**Limitations:** Settled screenshots are not frame-by-frame drag captures. This harness has no grouping hook and the two surfaces still move sequentially. Real Vesktop/Quickshell input masks and live startup are a separate later proof.

## WATCH OUT FOR
The render transaction paths (`sway/desktop/transaction.c`, `sway/desktop/output.c`, `sway/tree/view.c`) currently overlap active Company Stage Director drafts. **Do not modify them from this assignment.** The CI must run isolated, and a future compositor-level grouping change requires owner coordination, native actor custody, popup/seat gates, multi-output/scale verification, a rollback feature gate and actual same-frame screenshot/scene-frame evidence.

## NEXT
Use the real-window baseline to specify and verify the first **compositor-owned paired scene update**. Implement only under delegated Stage Director ownership; target a visible frame with no independent intermediary pose. Continue using Social W's current fast-follow bridge as the production fallback. No merge or workstation activation without operator approval.

## DECISIONS
The user's quality bar remains perceptually unified opening, dragging, and resizing. Success here is a real Wayland **baseline**, not completed zero-lag synchronization.
