# SwayPX // Native Middle-Click Autoscroll Core

> **STATUS // DRAFT, SOURCE-ONLY, NO RUNTIME HOOK.** This is a compositor-native alternative to the external evdev/uinput experimental driver in `The-Post-Apollo-Project`. It is not enabled, compiled into the shipping compositor, installed or activated.

## Patient / Room / Authority

PATIENT // `kudokudo1/Post-Apollo-SwayPx`

ROOM // Native Universal Autoscroll · private compositor core exploration

STAFF // External Hospital-compatible DOC. No live Hospital registration or compositor ownership assignment has been verified.

SOURCE BASE // SwayPX `master` at `2d91c5926b1a86679a64a53511667cba54a417c2`. Recheck `master` and active ownership before changes.

## Why the compositor can do this better

`sway/input/cursor.c` already processes the real `wlr_pointer_button_event` and has the trusted `cursor->cursor->x/y` layout coordinates. Its `node_at_coords()` performs actual scene-tree hit-testing, including popups, layers and no-surface cases. `sway/input/seatop_default.c` forwards axis events through `wlr_seat_pointer_notify_axis`.

This means an opt-in native mode could keep ownership in SwayPX instead of granting a separate process access to `/dev/input/eventN` and `/dev/uinput`. Avoiding that raw-device path would also avoid the external client's missing-pointer-origin proof problem.

## Implemented here

The isolated `post_apollo_autoscroll.c` and header provide:

- Explicit `mode_enabled` and genuine hit-tested target gates on session creation.
- Anchor coordinates and accumulated movement, with finite checks, 15 px dead zone, 1200 px displacement clamp, and 2800 px/s rate cap.
- Continuous scrolling intent on each timer tick even when the pointer has stopped moving.
- Cancellation on target change, output-layout generation change, expired/retrograde/stalled clock, nonfinite motion, or explicit stop.
- Session generation to prevent old-state reuse; no direct pointer, keyboard, seat, focus, process, IPC, or Wayland operations.

The C code is **not added to `meson.build`** and not called from `cursor.c`. It emits a bounded *distance intent*, never actual wheel events. Unit tests compile it in isolation. The test workflow explicitly rejects changes to active input dispatch or IPC during this stage.

## Proposed integration, requiring compositor owner acceptance

After this core passes CI and the owner accepts the work:

1. Introduce a configuration default **OFF** with exact physical-device/seat selection and a safe restore/disable path. Sway Scroll Lock and native autoscroll must be mutually exclusive.
2. In the existing middle-click handler, snapshot the actual cursor coordinates and `node_at_coords` result. Reject session lock, popups/layers/unmanaged/no-target, and non-physical or virtual pointers unless separately justified.
3. For the active session, compositor-owned motion should update displacement without moving the real pointer; a bounded Wayland event-loop timer should generate vertical/horizontal axis events via the established seat operation, not a uinput device.
4. Show an anchored, non-focus-stealing indicator through a compositor-authorized output-local surface, and cancel on second click, Escape, other click, target/output loss, session lock, disabled mode, device removal, compositor reload or service exit.
5. Exercise a disposable headless SwayPX session with real Wayland clients on multiple outputs, including Brave/Kitty/Zellij-like surfaces; complete independent security review before any operator-approved desktop trial.

No public global-pointer IPC, credential-granting udev rules or automatic autoscroll activation should be added merely to make a smoke test pass.

## Ownership and rollback

This PR is a **review-only component candidate** for the SwayPX owner. It does not change the compiled compositor, running desktop, SwayPX service, user mouse or CPU++ settings.

Rollback: close the PR or delete its branch. The current live engine is unaffected. Do not call the C math tests evidence of end-to-end compositor input handling.

## Verification

The dedicated `post-apollo-autoscroll-core.yml` compiles with `-Wall -Wextra -Werror -pedantic`, runs five groups of rate/safety tests, and verifies no runtime input-path changes in the feature diff.

## NEXT

Review whether a fully compositor-native mode should supersede the planned external raw-device implementation. The operator and SwayPX owner retain authority over changing live input dispatch, build wiring and production activation.
