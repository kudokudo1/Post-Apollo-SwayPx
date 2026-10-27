# Stage Director SD1 // Weather NEWS trusted-session integration handoff

**STATE //** Coordinated integration proposal and verified source inventory; **NOT** implementation approval, production activation, or a live Hospital Room/Assignment/Session.
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx` (native compositor policy)  
**NEIGHBOR PATIENT //** `kudokudo1/The-Post-Apollo-Project` (Weather UI + NEWS catalog/preflight)  
**SOURCE CHECK //** 2026-10-27 GitHub source on Weather UI `main` and Stage Director draft PRs #26/#27/#29.  
**OWNER //** operator, with separate UI/launcher, Stage Director, and input-security workstream review required.

## IMPLEMENTATION // Current verified state

### Weather host and UI: implemented in UI repository main

- [Weather PR #774](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/774) **merged**: `StationChannelScreen.qml` uses a crash-safe static WATCH view; the old in-process `StationNewsWebView.qml` remains unreachable because WebEngine startup previously aborted Quickshell. CLOSE returns to NEWS and mute defaults on/disabled.
- [Weather PR #777](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/777) **merged**: `scripts/weather/news_viewer_preflight.py` inspects known browser executables, validates enumerated HTTPS targets (or strictly typed YouTube IDs) and always returns `can_launch:false`, `can_play:false`, `state:BLOCKED_VIEWPORT_UNAVAILABLE`. It does not launch, talk to the compositor, write a profile, or use a session credential.
- [Weather PR #804](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/804) **merged**: `widgets/weather/WeatherStationW.qml` declares static `WlrLayershell.namespace: "post-apollo-weather-station"` before connection, `WlrLayershell.layer: WlrLayer.Overlay`, and a reactive **local** `newsViewerViewportRect` with `newsViewerGeometryValid`. The earlier claim that the namespace was missing is **obsolete**. Local QML coordinates alone do not supply the native layer ref, global logical coordinates, output generation, or session authority.
- `StationNewsHub.qml` owns catalog selection, `viewerOpen`, `chosenRecord`, `channelMuted` and `closeViewer()`. `StationChannelScreen.qml` owns an inert `CHECK VIEWER` helper and the player-bay rectangle. Opening WATCH sets UI state; it does **not** create a Wayland browser actor.

### Stage Director and renderer: development evidence only

- `sway/stage_director.c` shadow observer and `stage_director_weather_host.c` classify native layer candidates without granting permission. A matching namespace is **discovery**, not an authenticated session; a matching app_id/class is also **discovery**.
- `stage_director_news_custody.c` holds a candidate pidfd under matching PID/starttime/UID; `stage_director_news_supervisor.c` launches **only a fixed test child** with `SD_STAGE_DIRECTOR_TEST_HELPER`. Neither provides production NEWS browser startup or grants clip/focus/input.
- `stage_director_viewport.c` already validates host/output/actor/viewport geometry and a six-boolean authorization contract. Draft [PR #29](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/29) adds a **pure per-frame native owner decision**: matching epoch, host/actor/output/root-surface refs and fresh geometry are required; all authority gates must be present. Default on mismatch: no proposed destination override. No caller or production actuation exists.
- Draft [PR #27](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/27) contains the merged [PR #28](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/28) diagnostic correction: `output_configure_scene()` was overriding a 320×240 source-crop destination with the full `1398×845` content rectangle at every repaint. In **isolated CI only**, a compiled and environment-gated exception preserves the 320×240 destination. Real SceneFX/GLES trace and sampled left/top/right/bottom/corner pixels passed [run 40464912910](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40464912910) and were still green on [PR #29 run 40479077901](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40479077901). This is not authenticated browser containment or production actuation.

## HOW TO USE // Proposed handoff between independently owned seams

**1 // Weather UI owner: intent producer, never credential issuer.**

On explicit WATCH activation, the UI can provide one validated catalog key (`source` ID or strict YouTube resource type and ID), request OPEN, report a fresh bounded **local** player-bay rectangle and screen association, and display `BLOCKED/STARTING/VISIBLE/FAILED/CLOSING`. CLOSE/menu close/mode switch revokes the intent; MUTE remains true until a real controlled player reports effective audio state. Do **not** carry browser login tokens/cookies in QML or let QML nominate an arbitrary PID, process identity, native Wayland surface pointer, or a trusted boolean. Keep QtWebEngine out of Quickshell.

**2 // Trusted launcher/session owner: approval and process provenance producer.**

A separately authorized broker must check the enumerated target (no arbitrary URL/argv), confirm operator intent, allocate a unique strictly monotonic per-host session epoch, and launch exactly one known external browser process using explicit arguments and a private profile/session location. It must hold its **actual child process** and launch evidence (PID, start time, UID, pidfd or equivalent); link the child/browser Wayland connection and its mapped xdg_toplevel root surface to the intended launch, and reject unrelated browser windows. A pidfd by itself proves process identity, not operator intent or exact window origin. Multiprocess browser lineage, delegated child windows and portal/protocol handling require an explicit verified policy, not a guessed ancestry. A random request token, app_id or title alone is not a credential.

**3 // Native Stage Director compositor owner: host, geometry and output authority.**

Observe the mapped `post-apollo-weather-station` layer with its **native surface identity** and live output, and map the QML local viewport into global logical coordinates with output offsets/scale/transform accounted for. Join it to the broker-confirmed exact browser root surface under one session epoch. Validate `stage_director_viewport_prepare_clip()` and the PR #29 per-frame owner/observation decision **at every output repaint**. Any accepted source crop must have matching renderer destination width/height applied in the same frame, otherwise SceneFX will stretch it again. Normal SwayPX animated content sizing remains the default for every other view and after revocation. Avoid a second competing geometry writer.

**4 // Input/focus/popup owner: verified guard installer, not a preflight checkbox.**

Install and test pointer hit-test/coordinate mapping for the selected actor while rejecting every off-bay target, keyboard entry/focus handoff and release (including Quickshell OnDemand behavior), and rejection or confinement of xdg-popups/menus that may be rendered in a different Sway scene tree. Record actual installation and capability state; no user-supplied boolean can authorize it. No keyboard capture merely because NEWS is visible, and no cross-workspace yank to a browser.

**5 // Teardown authority shared by launcher/compositor/host.**

CLOSE, switch away from NEWS, host unmap, output loss/move, browser crash, invalid ancestry, duplicate host/browser, stale epoch or policy mismatch must revoke the same session, remove source/destination override and clip, release input focus and popup guards, restore standard SwayPX sizing, stop/reap **only the owned child** and preserve unrelated browser processes/profiles. If teardown is uncertain, fail closed and report it rather than grant a new epoch. Keep the safe static WATCH view as rollback until fully accepted.

## VERIFY // Required staged acceptance, each independent

1. **Source safety:** existing Weather SAFE MODE and `news_viewer_preflight.py` must keep `can_launch=false` before the authorized session broker exists. No dynamic WebEngine import to Quickshell or arbitrary URL/argv execution.
2. **Broker contract:** mock/isolated tests establish only an operator WATCH request can mint an epoch. Prove unique-host/source mapping, source allowlist, no PID reuse, browser child vs unrelated process discrimination, duplicate attempts, and no arbitrary session reattach.
3. **Native host binding:** mapped Weather namespace/overlay/output, native layer ref, local-to-global viewport math, scaled/multi-output layout and host disappearance are checked; no trust is inferred from namespace alone.
4. **Rendering:** real browser actor screen captures at start, resize, output change, move, CLOSE and crash; assert the entire actual paint region is confined on all four edges (not just hit-testing) and SceneFX destination stays tied to the crop at repaint time. CI terminal screenshots are only precedent.
5. **Input/popup:** pointer coordinates including borders/scroll/drag, keyboard/focus release, modal/dialog/popups, window/menu switching, overlay and workspace layering; no interaction outside bay and no unintended browser foreground focus.
6. **Process lifecycle:** CLOSE stops/reaps only the owned renderer; no duplicate windows/orphaned children; restart of shell/compositor and broker uncertainty retain deny state; mute starts on and returns actual effective state.
7. **Publisher/media reality:** allowed sources such as Congress, Fox, CNN and Al Jazeera have differing embedded playback/authentication policies; site catalog eligibility is not proof of streaming, DRM, or paywall access. Verify representative sources independently.
8. **Review/rollback:** retain static viewer, no production or local desktop modifications without operator authorization; do not merge stacked diagnostics or the pure policy as an implicit production activation.

## WATCH OUT FOR // Conflicts and unassigned decisions

- **Ownership transfer:** Stage Director SD1 browser-viewer integration crosses the Weather UI, process-lifecycle and input-security rooms. This document requests coordination; it does not assign those roles, register a Hospital Room, or authorize mass surgery. Operator approval and a verified shared work contract are needed before modifying a neighbor's files or activating a compositor.
- **Identity model unresolved:** which specific trusted broker can attest the browser Wayland connection and its root surface, including Chromium's multiprocess structure? Exactly how is the session epoch shared with compositor without accepting claims from untrusted IPC/QML?
- **Geometry model unresolved:** derive local-to-global bay on each frame/output, handle scaling/rotation and multiple Weather host instances; avoid stale layer references and output replay.
- **Window lifecycle unresolved:** how to represent transient menus, file dialogs and browser popups, and who owns input mapping and browser focus?
- **Diagnostic conflicts:** PRs #25/#26/#27 are draft branches, PR #28 merged into #27 development base only, and PR #29 remains pure-policy draft; compare diffs and merge bases before any integration. Do not use a developer CI app-id allowlist as a production trust primitive.

## CHECKLIST

- [x] Inspect current UI source, merged #774/#777/#804 and current compositor source
- [x] Confirm Weather native namespace and local player bay exist
- [x] Confirm isolated rendering correction and pure frame policy have green CI
- [x] Document owner responsibilities, default deny, revocation, acceptance and rollback
- [ ] Assign/approve joint host + launcher + compositor + input security integration ownership
- [ ] Select/authenticate broker and Wayland actor linkage
- [ ] Implement and test an independently authorized session in reviewed branches
- [ ] End-to-end real browser safety acceptance, then operator-controlled activation

## NEXT

At the next authorized integration boundary, build and test **only the trusted session broker/lease producer and compositor-native evidence binding**, while keeping UI WATCH safe mode. Separately integrate per-frame destination and input/popup guards only when an actual native session is established. Stop before any live runtime or production merge without operator approval.

## DECISIONS

No new ownership or canonical production contract is silently declared. GitHub and operator remain sources of truth; Hospital-compatible handoff only. This file is **documentation**, not a production actor, implementation permission, or a claim of live verification.
