# Stage Director // SD1 Weather NEWS viewport upgrade (V0 geometry/identity)

**STATE //** Implemented on an isolated SwayPX development branch, not compositor activation.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`. **BASE //** `stage-director/scene-registry-v1` (SD0).  
**OWNER //** Operator. Do not infer live Hospital assignment, acceptance, permissions or deployed binary from this source tree.  
**DEPENDENCY //** Post-Apollo Weather Station safe NEWS player plus an external process transport.  
**DATE //** 2026-10-24.

## IMPLEMENTATION

**Problem:** Quickshell 0.3.2 + Qt 6.11.2 aborts when `QQuickWebEngineView` constructs QtWebEngineCore inside the shell (missing argv/QCoreApplication argument initialization). Weather NEWS currently uses a safe-mode viewer and has a standalone browser-availability preflight, not an active browser player. The operator wants WATCH IN STATION → real browser video inside the NEWS bay → × CLOSE, without window jumps or risking a Quickshell-wide crash.

SD0 has an observer-only actor and layer registry on the `stage-director/scene-registry-v1` branch. It is **not** on SwayPX master. Its Weather profile matches Kitty `app_id=weather-screen` and declares a historic 780 × 416 transform. That declaration is not an authoritative current bay geometry.

This SD1 step adds genuine, compiled planning primitives while preserving SD0's nonmutating boundary:

1. `stage_director_profile_weather_news.c`: classify **only** a separate browser actor whose native Wayland app_id or XWayland class is exactly `post-apollo-weather-news`. No title matches, browser-brand guesses or navigation-dependent actor identity. Record scene `weather-news`, role `browser`; `anchor=false`, `can_lead=false` deliberately prevents the actor declaring its own scene ready. Actual launcher does not exist yet.
2. `stage_director_viewport.h/.c`: pure logical-geometry policy. Given a **verified** actor, live mapped host, active output, actor bounds, host bounds and requested viewport, fail closed for unverified identity, unmap, disabled output, invalid/overflowed rectangles, viewport escaping host/output or actor not covering the full viewport. On a valid request calculate the actor-local crop. Never interpret this crop as applied render clipping.
3. Native zero-dependency C test exercises viewport calculations, large/invalid rectangles, negative origins and secondary outputs, exact pointer boundaries, actor/host detach and fail-closed denial. Static test locks identity semantics and ensures no compositor mutation APIs were added to this module. CI compiles with `cc -Wall -Wextra -Werror -pedantic`.

**Crucially:** even when the crop is valid, `can_launch=false`, `can_render=false`, `can_accept_input=false`. This is a **V0 Stage Director upgrade**, not a promised functioning browser viewer. SD0 still only observes. Existing SwayFX, clipping, focus, window lifecycle, and current live desktop are unchanged.

## HOW TO USE

To inspect the dry-run contract, compile the standalone test from the branch:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
    sway/stage_director_viewport.c tests/stage_director/test_viewport.c \
    -o /tmp/sd-viewport-test && /tmp/sd-viewport-test
python3 tests/stage_director/test_observer_boundary.py
```

These commands **do not** start a compositor, move windows, open a browser or persist configuration. Use GitHub CI for official verification; do not execute `build.sh` or replace SwayFX on the live host without a separate operator-controlled deployment plan.

## VERIFY

V0 gate: exact external browser actor classified by a stable identity; output, host and viewport coordinates checked with bounds; crop coordinates correct for multiple outputs and safe at integer edges; invalid/unmapped actor and host denied; no renderer, focus, input or process activation authority. Complete independent compositor compile and live rendering remain **unverified** until a worker builds this branch with actual SwayFX/wlroots dependencies and tests under a nested compositor.

The branch is intentionally based on SD0, not on `master`; Git comparison shows SD0 and `master` have divergent histories. Do not merge SD1 into `master` as a shortcut around SD0 integration.

## WATCH OUT FOR

- **Layer identity:** Weather Station is a layer-shell `PanelWindow`, not a regular window. SD0 observes layer geometry but does not currently establish trusted association/authorization between the actual Weather Station host and a NEWS browser. This must be implemented before applying a crop.
- **Clipping:** wlroots/SceneFX scene-graph surfaces can have their own crop APIs, but clipping an entire independent XWayland/Wayland browser surface with popups, decorations and input remapping requires a separately verified implementation. An opaque rectangle drawn over overflow is *not* compositor clipping.
- **Multi-output:** Never assume DP-5/DP-3 names, global coordinate origin zero or a fixed 824x494 logical bay. The host and output geometry must be observed on the current Sway scene, including panel workarea offsets, scale changes, hotplug and teardown.
- **Actor identity is not authorization:** A process setting a matching app_id/class does not by itself prove process ownership or operator intent. Authorize an ephemeral launcher token/process association in the future bridge and fail closed on duplicates or cross-scene injection.
- **Audio/focus:** No active player means no mute control. Never claim MUTE affects an unseen browser or that hidden windows cannot receive pointer input without rendering+hit-test evidence.
- **Build:** This PR adds source entries in SwayPX Meson; standalone unit tests do not certify that a complete compositor or a replacement Sway binary builds and boots.
- **Other ownership:** The existing SwayPX Stage Director V0 and P-level Weather viewport PR remain separate work. Avoid rebase/copy/merge without inspecting ownership and notifying the operator.

## CHECKLIST

- [x] Inspect SwayPX master, SD0 branches, active PR #3 and Post-Apollo NEWS safe mode
- [x] Register specific external NEWS actor identity on SD0 branch
- [x] Implement overflow-safe standalone viewport crop/denial policy
- [x] Add executable C unit tests and static no-authority guard
- [x] Add isolated CI workflow
- [ ] Verify CI on pull request
- [ ] Build complete compositor in isolated environment
- [ ] Obtain live owner/scene/host association design and explicit activation gate
- [ ] Implement opt-in compositor clipping with input hit-testing and rollback
- [ ] Test isolated browser video, mute, popup handling and CLOSE under nested SwayPX
- [ ] Operator authorizes merge and any local deployment

## NEXT

**SD1 V1** should be a separate, reviewable compositor integration after SD0 lands: a trusted layer-shell host + owner token handshake; an explicitly authorized isolated browser actor; a clip enforced by compositor rendering **and** pointer hit-testing; one-actor lifecycle and fail-closed teardown. First prove on a nested compositor, then verify dual-output geometry with the operator, and only then wire the NEWS transport to permit an external browser launch.

## DECISIONS

- Prefer Stage Director containment over crashing in-process QtWebEngine.
- Build atop SD0, not a parallel scene registry.
- Treat pure viewport geometry as a necessary gate, never as proof of visible clipping.
- Do not silently promote P-level viewport ideas to live authority or merge into SwayPX master.
- Keep everything reversible and live SwayFX/Quickshell untouched until explicitly authorized.
