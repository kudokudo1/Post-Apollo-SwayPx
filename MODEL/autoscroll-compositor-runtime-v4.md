# SwayPX // Native Autoscroll V4 · Compiled Cursor Integration, Hard OFF

> **STATE // REVIEW-ONLY DRAFT, ZERO LIVE INPUT ACTIVATION.** This change compiles a new private autoscroll runtime into the compositor and calls it from actual cursor input paths. Its hard gate is `PA_AUTOSCROLL_LIVE_ACTIVATION 0`. No code or UI can turn it on.

## Hospital contract

PATIENT // `kudokudo1/Post-Apollo-SwayPx`

ROOM // Native Universal Autoscroll · Cursor Runtime V4

STAFF ROLE // External Hospital-compatible DOC, not a verified live registered worker or Company Map owner

ASSIGNMENT // Operator reports the Company Map worker is **paused**, reducing concurrent activity. That does **not** automatically transfer ownership. Changes are isolated in a draft based on PR #141 and must not merge or deploy before explicit shared-seam acceptance.

LATEST SOURCE BASE // `master` `2d91c5926b1a86679a64a53511667cba54a417c2` and V3 head `f37dadf2dd8850c457121bcbc8307223d0037f7f` at preflight; recheck both before merge.

PARALLEL ROOM // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126), paused worker, **open draft**. It edits `sway/input/cursor.c`, `include/sway/input/seat.h`, `sway/input/seat.c`, and `sway/meson.build`. Do not merge or rebase blindly over its branch.

## IMPLEMENTATION

- `sway/input/post_apollo_autoscroll_runtime.c` adds the real compositor-side bridge for the existing V1 speed core, V2 middle-click adapter, and V3 axis dispatcher.
- The actual `sway/input/cursor.c` now invokes the bridge on relative pointer motion and mouse button press/release **after** both Stage Director Weather/Company physical-input pairing guards. When hard OFF, those calls return `false` and the original code continues unchanged.
- `sway_cursor_create` calls an optional factory that returns `NULL` while the production hard-OFF gate is set. **No per-cursor heap allocation or Wayland timer is made when disabled.** If a future approved test build enables the gate, the factory allocates the runtime/timer and destruction cancels/frees them. Failure to allocate that optional bridge leaves normal mouse input working.
- Meson compiles the four namespaced autoscroll sources into the compositor.
- A potential axis sink uses `wlr_seat_pointer_notify_axis` and `wlr_seat_pointer_notify_frame` with pixel-scroll semantics. **It cannot execute** because the compile-time guard is 0, the adapter is never enabled, the verified physical-source flag is false, and missing native Scroll Lock/output-generation evidence forces refusal.
- No IPC, public pointer coordinates, udev permission grant, external input grab, compositor configuration option, keyboard interception, production service, or local desktop changes.

## VERIFY

- `tests/autoscroll/test_default_off_compositor_hook.py` checks build inclusion, hard-off activation, Weather/Company ordering, pointer passthrough and absent privileged hooks.
- GitHub's autoscroll workflow compiles V1 rate, V2 adapter and V3 axis tests with warnings as errors, plus source contract.
- A **full SwayPX build** (if triggered on these paths) is required before treating the bridge as compiled and linkable. Real pointer-event and output semantics still require headless SwayPX tests, and this PR is not proof of Windows autoscroll usability.

## WATCH OUT FOR

There are deliberately three independent gates to prevent premature activation: hard-off compile constant; no enable call; missing verified physical-pointer/Scroll Lock/output-generation data. They must not be bypassed merely because the file builds.

The Sway Scroll Lock alternative in CPU++ remains separate. It may be active per physical device, so future native autoscroll must not consume middle-button while that Sway mode owns it.

## NEXT // Shared owner acceptance and headless runtime integration

The next change is a gated integration that verifies exact physical device identity, pointer-under-cursor target and output-layout generation, implements the compositor-owned session timer and Escape binding, draws an input-transparent anchor, and tests live Wayland axis events with a disposable SwayPX compositor. It requires owner review and operator approval before activating any real desktop. No PRs are merged by this stage.

## RECOVERY

This branch is review-only. Closing the draft or reverting its commits removes the source changes. There is **nothing to undo on the operator's machine** because no local files or running compositor were changed.
