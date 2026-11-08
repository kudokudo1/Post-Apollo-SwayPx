# Stage Director // Provider-neutral native window groups (proposed v0)

**STATUS // PROPOSED, NOT IMPLEMENTED**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**CONSUMER //** `kudokudo1/The-Post-Apollo-Project` / Social W  
**SOURCE //** real Social W / Vesktop dock at Sway IDs 75 + 53, 2026-11-08; production Social grouping [PR #109](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/109), exact Flatpak app-ID correction [draft PR #114](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/114).  
**OWNER BOUNDARY //** SwayPX owns compositor-native identity, group lease, seatop and scene transactions. Quickshell owns explicit Social UI selection, provider launch intent, dock geometry and per-provider identity isolation.

## Problem / why separate from the Vesktop fix

Merged `stage_director_social` is an opt-in production compositor command for exactly one Social `QS_SOCIAL_MENU` floating XDG host and one native Wayland Vesktop guest. Its one-group move/resize hooks are proven in a disposable headless compositor. In the actual Fedora session, Vesktop advertises `dev.vencord.Vesktop` rather than the lowercase IDs previously admitted, causing a real pair rejection; PR #114 adds that *exact* spelling and its regression case. No claim of operator live acceptance follows from CI.

The requirement for more than Vesktop is **not** satisfied by relaxing `exact_guest()` to accept any title/app ID. Other providers may be XWayland, a browser with provider-profile isolation, independent native applications, or content already inside Social's QML tree. Different actor classes require different adapters and explicit authority.

## Existing consumer classes (verified from merged Post-Apollo sources)

| Consumer | Current integration | Provider-neutral next gate |
| --- | --- | --- |
| Vesktop / Discord | Exact XDG guest and Social dock; special `stage_director_social` command | Preserve existing path and fallback; PR #114 fixes observed capital-V app ID |
| Sessions + Telegram | Rendered within Social's Quickshell/QML UI | No separate external native pair required |
| Managed social sites (X, Element, Reddit, others) | Post-Apollo browser classes and geometry helper; scoped identity classes may exist | Identify real compositor shell type + exact con_id and profile custody first; XWayland remains fallback |
| Signal / Viber / WeChat | Explicit `DesktopServiceLauncher.py` requests independently owned desktop clients | Provider app ID, shell type, single main toplevel, account/profile ownership and explicit user Dock intent **not verified**; do not auto-admit |
| PHONE setup tool | Separate dedicated terminal actor and input cutout flow | Owned by PHONE integration; no implicit reuse of Social native pair |

Sources: `widgets/messanger/DiscordGeometry.py`, `widgets/messanger/MessagingW.qml`, `services/messaging/DesktopServiceLauncher.py` in `The-Post-Apollo-Project`. These describe *current code*, not guaranteed installed-app capabilities.

## Proposed native group contract (not an available command)

1. **Request identity**: an explicit Social `Dock` gesture chooses a provider-specific candidate. The helper resolves exactly one live Sway container, not a window name search or saved account nickname. No app is launched/paired on menu hover, selection alone, or account switching.
2. **Compositor admission**: a future opt-in `stage_director_group` interface takes exact Sway container IDs and a *compositor-recognized role/slot policy*. It independently checks the current mapped native XDG views, distinct `wl_client` and root surfaces, positive PIDs, same workspace, accepted host/guest roles and bounds, floating nonfullscreen state, and docked geometry before granting a generation-scoped group. A client-supplied app ID or policy name is never its own authorization.
3. **Authority**: same-user Sway IPC already controls windows, so this interface does **not** independently authenticate the human operator. Cross-UID or remote authority remains excluded. Do not expose raw native pointers to Quickshell, providers or network clients.
4. **Lifecycle**: revoke upon exact guest/host unmap, replacement, destruction, selection/profile change, concealment, workspace/output mismatch or explicit clear. Existing native seatop hooks may only move/resize the still-valid matching generation and must leave unrelated windows unchanged.
5. **Rendering / input**: selected external guest must actually paint above the opaque Social chassis, take only intended pointer/keyboard input, and preserve focus/popup bounds. Matching Sway rectangles alone do not prove pixel or input correctness.
6. **Failure**: unsupported or failed native admission leaves the current provider-specific legacy bridge or independent window behavior intact. Never hide a provider or suppress follower geometry solely because a pair was *requested*. Old `stage_director_social status|clear|pair` remains backward compatible until an explicitly verified migration.

The proposed interface name and payload are **open design questions**, not permission to alter shared `sway/commands.c`, seatops, transaction, rendering or native protocol machinery. Reuse existing Social generation and view-unmap protections rather than creating a second inconsistent lease model.

## Minimum implementation path and acceptance

**A. Observe (read-only):** after the Vesktop correction is accepted, capture one candidate non-Vesktop client with actual `swaymsg -t get_tree` identity, shell, focus, workspace and geometry; verify the user selected Dock. Record whether that actor is native XDG or XWayland. Do not guess provider app IDs from command names.

**B. Isolate policy:** introduce reviewed, testable provider/role policy logic with no production actuation, covering one accepted native non-Vesktop candidate and denials for wrong provider, stale con_id, duplicate app IDs, wrong profile, hidden/splash, XWayland, cross-workspace, same `wl_client`, fullscreen and unauthenticated lifecycle changes. Coordinate with the existing Company / Weather Stage Director owner before modifying shared code.

**C. Production implementation:** reuse the existing native grouping state machine and seatop transaction hooks, keep the legacy Social command compatible, and add one new explicit provider adapter only after A and B pass. The app adapter lives in the Post-Apollo Project and owns UI Dock/Undock intent and per-identity isolation; compositor authority stays in SwayPX.

**D. Verification:** an exact-head production build; a disposable real Wayland two-provider pixel/drag/resize test; old binary fallback, unmap/replacement and cross-app denial; real operator physical Mod-move, resize, fullscreen, output-scale, popup/focus, scratchpad and sign-out tests. No CI mock is user visual acceptance.

## Watch-outs and handoff

- This document is an architectural candidate. It grants **no new native grouping authority**, does not modify the compositor and cannot make Signal, Viber, WeChat or web apps suddenly dock.
- Current live SwayPX `f5abef8a` remains the installed binary until a separately authorized operator deployment. PR #114 is a separate, narrower bug fix.
- Current open Company M30s draft [#113](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/113) owns independent headless Company/MapLibre CI. Shared Stage Director core expansion requires explicit collision review.
- No local Quickshell edits, compositor restart, package installation, Git merge or production activation are authorized by this proposal.

## Decision state

- **Verified:** current production native pairing is Vesktop-specific; the Social implementation supports several different external integration types.
- **Proposed:** a provider-neutral, native-XDG-only, opt-in group contract with compositor-owned identity and generation-scoped revocation.
- **Not decided:** first non-Vesktop candidate, app ID policy, command shape, cross-provider scene/input semantics, and worker owning shared compositor integration.
- **Recovery:** retain merged PR #109, draft bug fix #114, the existing Social 8ms fallback, and this contract as a separate review-only pointer. Operator retains final acceptance.
