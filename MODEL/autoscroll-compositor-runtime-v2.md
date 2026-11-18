# SwayPX // Native Autoscroll · Input Adapter V2

> **SOURCE-ONLY · DEFAULT OFF · NOT LINKED INTO COMPOSITOR.** This branch depends on native-engine draft PR #139. It introduces no changes to `cursor.c`, `seat.c`, `seatop_default.c`, `ipc-server.c`, `meson.build`, keyboards, overlays, host permissions, or production startup.

## Hospital state

PATIENT // `kudokudo1/Post-Apollo-SwayPx`

ROOM // Native Universal Autoscroll / private event adapter V2

STAFF ROLE // External Hospital-compatible DOC; compositor owning worker is not established, no live Hospital registration

ASSIGNMENT // Continue operator-requested universal middle-click autoscroll, but prepare trusted input contracts separately from shared compositor hooks. Source base is PR #139 commit `6006ed595188c81472c2b2b1bbdbef98cc154563`.

## What the code actually does

`include/sway/input/post_apollo_autoscroll_adapter.h` and `sway/input/post_apollo_autoscroll_adapter.c` translate *hypothetical compositor-authoritative events* into calls to the Stage 1 math engine:

| Input | Default OFF | When explicitly enabled by future compositor owner |
| --- | --- | --- |
| Physical middle-button down/up | Passes through untouched | Starts anchored mode on validated hit-test; consumes press and paired release; second middle-click cancels |
| Mouse relative movement | Passes through | Accumulates bounded displacement without moving cursor |
| Timer tick | No event | Requests scroll pixels from the trusted native engine, even when stationary, via an abstract callback |
| Other left/right click | Normal | Cancels active mode and consumes press+paired release so the click does not activate an unintended control |
| Escape (same seat) | Normal | Cancels active mode |
| Session lock, Scroll Lock collision, device/seat mismatch, invalid pointer target, output layout change or stale tick | Normal | Cancels/refuses scroll output |

A bridge instance must be explicitly configured; after initialization it is **OFF**. Physical-source and pointer-target flags are **caller requirements**, not authentication supplied by arbitrary clients. Future compositor-owner integration must derive these facts directly from SwayPX's existing `wlr_pointer_button_event` and `node_at_coords`, not from keyboard focus or application JSON.

The adapter's `pa_autoscroll_axis_sink` is a dependency injection callback used only by isolated tests. No real `wl_pointer.axis`, uinput injection or event-loop timer is wired here. A future owner must verify the proper SwayPX axis dispatcher and client semantics on a disposable compositor, including high-resolution pixel vs detent units and interaction with mouse bindings.

## Critical stop and recovery properties

- Engine timeout: 180 seconds max, 100 ms maximum interval between ticks.
- Target, layout generation and physical device/seat are bound at activation.
- A swallowed button press must not leak its release, including after cancellation or disabling the mode.
- The adapter disallows activation while Sway's other native Scroll Lock mode is active.
- No globally listening keyboard hook is created; Escape is only a mock event until the compositor owner accepts integration.
- Unsupported layers/popups/session lock and absent pointer hit-test must remain refusal cases in any eventual runtime.

## Verification

`tests/post_apollo_autoscroll_adapter_test.c` runs eight test groups through a fake axis sink and synthetic compositor event identity. The dedicated GitHub C workflow compiles the Stage 1 math engine plus this adapter with `-Wall -Wextra -Werror -pedantic`. The workflow rejects modifications to shared SwayPX runtime input files.

The test is evidence for safe *state transitions* only. It is not proof of actual Wayland scrolling or visual compatibility in Brave/Kitty/Zellij.

## NEXT // Owning compositor acceptance required

The next source change is a deliberately small **opt-in, default-disabled compositor connection**. It would touch SwayPX's `cursor.c`, `seatop_default.c` and associated Meson wiring, so the SwayPX owning worker must explicitly accept that seam first. Then a headless real compositor test must verify pointer anchoring, scene target under the mouse, geometry/scale, axis semantics, Escape behavior, coexistence with existing Stage Director input pairing, and a non-focus-stealing indicator.

**No merge or production activation from this review-only PR.** Rollback is closing the draft branch; no runtime state was changed.
