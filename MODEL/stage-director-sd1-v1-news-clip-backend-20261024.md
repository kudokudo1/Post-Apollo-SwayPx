# Stage Director SD1 V1 // Weather NEWS compositor clip backend

**STATE //** Isolated implementation, **dormant and unmerged**. This is a real compositor-native clipping primitive, not verified end-to-end containment or an active external-browser viewer.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`. **BASE //** Stacked on SD1 V0 draft PR #4, itself stacked on SD0 observer branch `stage-director/scene-registry-v1`.  
**OPERATOR //** Final authority; no live Hospital worker, Room, Assignment or permission transfer assumed.  
**DATE //** 2026-10-24.

## IMPLEMENTATION

The operator requested a real NEWS browser in Weather Station without an out-of-station browser switch. QtWebEngine inside Quickshell crashed the whole host; the UI is on a crash-safe slate while SwayPX learns to contain an isolated browser.

Stage Director SD1 V0 already models exact browser identity and validates the viewport against mapped host, actor and output geometry. V1 now adds:

- `stage_director_viewport_prepare_clip()`: a separate authority gate and root-wl_surface coordinate translation. Geometry alone is NEVER activation. Every gate must pass: operator enabled, trusted host binding, browser process association, pointer hit-test guard, popup guard and teardown guard. All fields default false. Convert a valid actor-local crop to actual wl_surface coordinates, allowing nonzero XDG geometry offsets, with int64 overflow checks and untrusted dimensions denied.
- `stage_director_viewport_clip_view()`: compositor-native application through **`wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &box)`**. The method rejects unmapped/destroying views, absent content, mismatched browser app ID/class, stale scene origin or mismatched view geometry. There is no call site from SD0 or NEWS, no commands, no external process launch and no automatic activation.
- `stage_director_viewport_restore_view()`: explicit safe rollback while mapped through Sway's own `view_center_and_clip_surface()`. **Never reset clip to NULL**: ordinary SwayFX layout may require a clip and may overwrite the Stage Director crop during transactions.
- Register the adapter in the compositor Meson sources, add native C policy tests for every authority denial, XDG-root offsets, second output/negative origins and overflow, plus static source checks that the real wlroots adapter remains disconnected from the SD0 runtime.

**What this does not do:** it does not yet clip a live browser automatically. No trusted host ↔ actor session can currently set these gates. No pointer/keyboard hit-test remapping, popup denial, render ordering, one-actor lifecycle, input focus or process identity attestation has been implemented or activated. No compositor or Quickshell binaries have been built locally, replaced or restarted.

## HOW TO USE

Code review / standalone policy tests only:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
    sway/stage_director_viewport.c tests/stage_director/test_clip_policy.c \
    -o /tmp/sd1-clip-policy-test && /tmp/sd1-clip-policy-test
python3 tests/stage_director/test_clip_adapter_boundary.py
```

These commands run no compositor and launch no video. On the operator's desktop, continue to use the Weather NEWS safe-mode catalog; do not treat `CHECK VIEWER` as playback authorization.

## VERIFY

The standalone native tests and static no-activation guard run under `.github/workflows/stage-director-viewport-contracts.yml`. Passing these checks proves only logical crop and authority behavior plus presence of the wlroots clipping call in an isolated adapter. It is **not** proof that the entire SwayPX build succeeds against its installed wlroots/SceneFX ABI, that an actual browser renders clipped, or that input and popups are contained.

Rendering evidence must be obtained using a **nested compositor**. Verify browser actor position and layer-shell geometry from compositor state, open more than one video site, test oversize content, browser popup creation, monitor origin changes, hotplug, fractional output scale, focus, close/reopen, process crash and rollback. No live SwayFX or Quickshell modifications without separate operator authorization.

## WATCH OUT FOR

**Critical: Sway's existing transaction and view code also calls `wlr_scene_subsurface_tree_set_clip()`.** It can overwrite any new Stage Director crop at any commit/arrange, so the future session owner must install a post-layout reapply hook and restore the original Sway clip on detach. Do not add an unconditional reapply from SD0: it would mutate compositor state on mere observation.

Other limitations:
- **Layer-shell host:** The Weather Station Quickshell host is a layer, not a normal sway_view. SD0 observes its geometry, but it does not yet bind that actual mapped layer to a specific NEWS browser session or authorize viewport coordinates.
- **Popup escape:** XDG popups may be parented in a different scene tree, XWayland override-redirect windows may be unmanaged. The subsurface clip does not prove their containment. Reject popups and install input filtering before activation.
- **Identity spoofing:** `app_id=post-apollo-weather-news` is only an identification hint, not proof of a browser process owned by the Station. Match the one authorized child process/session, not any window claiming the identifier.
- **No automatic authority:** an all-true Boolean struct should be constructible only by the future trusted compositor-side session manager; no user-supplied geometry or JSON should set it directly.
- **Scene origin:** crop math requires the measured scene origin for the view's content tree. A stale/displaced actor rectangle is rejected.
- **SwayFX master ownership:** V1 is stacked on unmerged PR #4 and SD0, not ready to go into `master` without the prerequisites. SwayPX draft PR #3 is a separate strong design proposal.

## CHECKLIST

- [x] Inspect current SD0, V0, pending PRs and SwayFX native clip paths
- [x] Implement guarded native wlroots clipping and normal-view restore
- [x] Test root-surface coordinate translation / denial policies
- [x] Keep SD0 observer and production behavior unchanged
- [ ] Standalone CI green
- [ ] Full SwayPX compiler / linker validation
- [ ] Implement real trusted host/browser association, popup/input guards and post-layout reapply
- [ ] Isolated nested-browser runtime clipping evidence
- [ ] Operator authorizes merges and separate live deployment

## NEXT

**V1 follow-up: trusted scene binding and nested compositor proof.** First introduce a compositor-side session owner for exactly one Weather NEWS actor and one mapped Weather Station layer-shell host. Do not accept caller-supplied `host_session_verified` or `operator_enabled` as evidence. Install popup and pointer rejection gates, add an after-transaction crop reapply, close/restore on teardown, then test with a browser in an isolated nested compositor. Only after this can the NEWS `WATCH IN STATION` control attach to a launchable browser process.

## DECISIONS

Reuse the existing wlroots/SwayFX subsurface clip API, preserve regular window clipping on rollback, and leave this adapter dormant until a trusted Stage Director owner can **prove both paint and input containment**. Do not trade shell stability for an invisible window-management shortcut.
