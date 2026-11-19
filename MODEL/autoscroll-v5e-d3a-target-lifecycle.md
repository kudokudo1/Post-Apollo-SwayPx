# V5E-D3A // Real Mapped Window Target Lifetime, HARD OFF

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Mouse & Keyboard / Universal Autoscroll
STAFF // External Hospital-compatible DOC, not verified live-registered
ASSIGNMENT // Bounded D3A target/view object lifetime integration
BASE // V5E-D2 PR #153, exact head 86b50cdc9afeab2f4e50f23607d417ba2e99fc3a
OPERATOR AUTHORITY // Continue narrowly split shared-input development while Company Map #126 owner is paused; no takeover, merge or production authorization.

## IMPLEMENTATION

A mapped window in SwayPX is represented by a sway_container with a non-null view. The view_map path sets its surface before container_create(view); view_unmap invokes container_begin_destroy(view->container).

This child adds one typed PA_LIFETIME_TARGET slot to the real sway_container structure, mints it from the existing input-manager issuer after the successful container construction but before publishing the new node, and revokes it as the first action in container_begin_destroy, before close IPC, node-destroy signal, focus/fullscreen rearrangement or the delayed final free.

Split containers created without a view and containers without a mapped view surface never receive a token. The private read-only accessor reports ZERO for null, destroyed, unmapped or revoked targets. The accessor is not a hit-test, a surface verification, a guarantee that the view is visible, or authority to deliver events.

## HOSPITAL OWNER SPLIT

The new target-owned production paths are sway/tree/container.c and include/sway/tree/container.h. **Neither is changed by Company Map #126.** This child does NOT touch cursor.c, seat.c, seat.h, seatop_down.c, Company scene or popup/xdg handling, transaction.c, Meson, native output or existing autoscroll runtime. Inherited CI allowlists may be updated on this child branch only to recognize precisely scoped target additions; parent PRs are untouched.

V5E-B #149 retains its separate, previously notified cursor/meson overlap; V5E-C #151 and D1 #152 have a Meson-only overlap; D2 #153 has a seat.c/seat.h overlap. This D3A child does not create any new Company shared source collision. The Company owner will receive a discoverable GitHub comment linking the exact D3A child and its zero-overlap report.

## VERIFY

Dedicated CI checks strict changed-file constraints, production hard-OFF gates, source callback sequence, mapped-view-only token creation, token revocation on begin-destroy before signal or close, prior D2/D1/source safeguards, lifetime portable C11 tests and actual SwayPX Nix binary build.

**Not tested by this branch:** runtime XDG map/unmap live token trace, the target under a different surface hit-test/scene focus, layer-shell/popups/Xwayland unmanaged targets, output disable/re-enable and layout-change epochs, genuine physical input provenance, live autoscroll axis/overlay or Escape.

## WATCH OUT FOR

A container target token proves only its object creation and invalidation. This is intentionally insufficient for physical pointer origin, current surface under the pointer, scene/output ownership, UI focus or operator consent. The runtime keeps PA_AUTOSCROLL_LIVE_ACTIVATION 0, physical_verified=false, native_scroll_lock_active=true and layout-generation 0, and cannot consume middle clicks.

Do not infer that a valid token survives output moves, workspace changes, hidden surfaces or compositor restarts. A trusted consumer must check the full current scene, output and session identity.

## HOW TO USE

Review draft PR and exact-head CI only. No local desktop/Quickshell/SwayPX action.

## CHECKLIST

- [x] Re-read canonical Syntagma and current source/owner state
- [x] Map real view_map/unmap to container create/begin_destroy
- [x] Add target slot and internal read-only accessor
- [x] Preserve all Company source files untouched
- [x] Add separate exact-head source and native build CI
- [ ] Exact-head checks all green
- [ ] Company #126 durable zero-overlap notice
- [ ] Independent review and operator merge approval
- [ ] D3B output enable/disable/destroy and D3C layout invalidation
- [ ] Actual native lifecycle observation and strong physical origin proof
- [ ] Gated compositor-generated axis/frame, Escape/indicator and desktop acceptance

## NEXT

After D3A passes, put D3B in a separate stacked output-owned draft. D3B should reject inactive, disabled, destroyed and fallback outputs, issue fresh epochs on re-enablement, revoke on disable *before* workspace migration, and revoke again on begin-destroy. D3C must handle actual geometry/layout generation changes and Stage Director/Company exclusions.

## DECISIONS

Draft/unmerged, no live activation, no local host writes, no Company owner replacement or silent shared-seam changes.
