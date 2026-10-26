# Stage Director // NEWS pointer and popup containment gate V0

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`.  
**STATE:** Isolated feature branch stacked above `stage-director/news-supervised-helper-v0-20261026`. Candidate geometry only; no live compositor input interception, clip activation or NEWS browser playback.  
**AUTHORITY:** Operator final. No live Hospital Room, Assignment, staff ownership or deployment presumed.

## IMPLEMENTATION

The operator wants a separate browser playing inside the Weather Station NEWS bay. SD1 already has a dormant wlroots clip call, exact host/actor classification, process custody, fixed-source browser arguments and a CI-only supervised benign child. The remaining compositor safety issue is that **the paint surface is not the only interaction surface**.

I checked the source paths in the actual SwayPX development branch:

- `sway/input/cursor.c::node_at_coords()` uses `wlr_scene_node_at` to resolve a surface and its owning container/layer/popup.
- `sway/input/seatop_default.c` invokes that resolver for button, pointer motion, touch, and tablet flows, then may enter/focus the resolved surface.
- `sway/desktop/xdg_shell.c` builds XDG popups under `root->layers.popup`, separate from `view->content_tree`. `sway/desktop/transaction.c::arrange_popups()` positions that tree.
- `sway/stage_director_viewport_clip.c` clips only the main view content subtree and remains disconnected from transaction and input events. **That clip does not prove either pointer input restriction or popup containment.**

This increment introduces the reusable, pure `stage_director_news_containment.h/.c` contract:

1. **Pointer candidate:** Given a *future trusted compositor snapshot* (native host+actor references, current session epoch, actor identity, mapped host/actor/output geometry), check a fractional global logical pointer coordinate against the NEWS viewport. Accept the minimum edges; reject upper/right/bottom boundaries, NaN, infinity, foreign native actor, stale epoch/host or displaced geometry. Never round fractional output coordinates into the bay.
2. **Popup candidate:** A popup's **complete** global logical bounds must be strictly contained within the NEWS viewport; otherwise return `POPUP_OVERFLOW`. Reject invalid/overflowed rectangles, wrong actor/session, unmapped host and stale output. This is intentionally stricter than ordinary popup unconstraining: until real compositor popup handling exists, escaping a single pixel is unacceptable.
3. **No actuation:** Even if every geometric and claimed session condition passes, `can_dispatch_input=false`, `can_show_popup=false`, `can_enable_clip=false`. These read-only probes do not register a seat filter, hide any popup, move a view, set focus or grant trusted process ownership.
4. **Tests:** C11 unit tests exercise correct/foreign actor, epoch replay, fractional edges, invalid float coordinates, second monitor with negative global origin, full-box popup edges/overflow, invalid popup rectangles, and changing output/host. A source-boundary test ensures no runtime call sites from input, popup, clip or SD0 observer and notes the actual relevant Sway routing points.

## HOW TO USE

GitHub-only code review/test. Run the Stage Director viewport safety workflow on the stacked PR. No browser, SwayFX nested session, or operator machine action is implied.

The UI remains unchanged: NEWS WATCH remains a safe-mode screen. The pure helper cannot be invoked through any current NEWS button or public IPC endpoint.

## VERIFY

GitHub contract CI compiles `sway/stage_director_viewport.c`, `sway/stage_director_news_containment.c` and `tests/stage_director/test_news_containment.c`. Its static test confirms `node_at_coords`, `wlr_scene_node_at` and the separate XDG popup tree are still in the source and none calls the new candidate helper.

These tests **do not prove actual input containment**, popup hiding, compositor scene paint behavior, keyboard focus safety, or that an isolated SwayPX build links against its wlroots/SceneFX ABI. The render path remains unactivated. A green CI is not a live viewer.

## WATCH OUT FOR

- **Painter versus input:** restricting a tree's drawing is not equivalent to blocking `wlr_scene_node_at`, touch, keyboard, pointer constraints, relative pointer, scroll, drag, tablet, focus or popup scene paths. A future authenticated owner must enforce those at actual compositor event boundaries.
- **Popup geometry:** the helper assumes global *logical* popup coordinates from the compositor. It does not reposition or hide popups. A future implementation must choose either to destroy/refuse overflowing popups or enforce real popup render and input clipping, and must handle nesting and XWayland unmanaged surfaces.
- **Session spoofing:** passing `session_owner_verified=true` is not authentication by itself. These are candidate probes. Do not expose the snapshot struct directly to client-provided IPC or treat `CANDIDATE_ONLY` as permission.
- **Stale view:** the caller must invalidate any viewport on resize/move/output hotplug/unmap and recheck measured native scene origin before each event. This module verifies only its supplied snapshot, not a live wayland scene.
- **Keyboard focus:** no current V0 API gates keyboard, XDG activation, synthetic inputs, clipboard or browser pointer grabs. Treat these as separate security checks before exposing a functional WATCH.
- **SwayFX transaction clipping:** normal Sway `view_center_and_clip_surface()` and `transaction.c` rewrite the wlroots clip. The dormant clip adapter must be reapplied safely at a verified post-layout boundary and restored on CLOSE.
- **Dev only:** Development branches are not equivalent to `master`. No local SwayFX deployment is performed here.

## CHECKLIST

- [x] Verify PR #12 was merged along the development chain and inspect current source
- [x] Identify actual compositor hit-test, seat and popup code paths
- [x] Implement strict fractional pointer bounds and full popup rectangle policy
- [x] Add C tests and static no-activation regression checks
- [x] Register in Meson and GitHub workflow
- [ ] CI green
- [ ] Compile full SwayPX binary with real wlroots/SceneFX dependencies in isolation
- [ ] Authenticated native host/browser session and actual mapped NEWS viewport
- [ ] Integrate input routing and popup protection with compositor scene lifecycle
- [ ] Nested SwayFX rendering/input/teardown test, including multi-output scaling
- [ ] Operator explicitly approves merge and later local activation

## NEXT

The next substantial step is **full isolated compositor build and nested rendering/input acceptance**, not another independent launch token. First prove the existing dormant clip adapter can compile/link and accurately crop an isolated test surface. Then implement real, session-scoped routing guards at `node_at_coords` and the popup creation/unconstraining path, coordinate with keyboard focus and teardown, and test a disposable browser child in nested Wayland. Only after acceptance should NEWS WATCH be allowed to spawn the registered browser.

## DECISIONS

No changes to `cursor.c`, `seatop_default.c`, XDG popup routing or any production scene graph during this bounded policy milestone. All decisions are reversible. Do not equate geometry validation with verified containment, and do not start an uncontained browser.
