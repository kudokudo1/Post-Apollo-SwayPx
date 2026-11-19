# SwayPX // V5D Engine-Computed Axis to Native Wayland Receiver

> **STATE:** Disposable CI-only transport proof. Real autoscroll math, **virtual protocol** sender, no physical source authority, production HARD OFF. Not a compositor-native autoscroll enablement.

## HOSPITAL

PATIENT // Post-Apollo-SwayPx
ROOM // Mouse & Keyboard · Universal Autoscroll / V5D Engine-Wayland Test
STAFF ROLE // External Hospital-compatible DOC, no verified live Hospital registration or shared cursor ownership
ASSIGNMENT // Operator requested continuation: advance actual engine-to-Wayland receipt evidence without interfering with the paused Company Map cursor ownership.

BASE // Separate branch stacked on V5C-A/B PR #146 exact tested head b733d1355a3db4048bcf927424949f780a5db160. Changes only five new test/CI/documentation paths; none of #146's existing changed files.

## IMPLEMENTATION

- The new CI-only C sender compiles the actual SwayPX V1 speed engine, V2 click/motion adapter and V3 axis/frame dispatcher together.
- It initializes the engine in an independent synthetic fixture, explicitly enables only that test process, starts via fabricated trusted middle-click facts, moves synthetic displacement, and asks the real engine for two 16-ms ticks.
- The axis callback sends each engine-computed vertical pixel delta via the zwlr_virtual_pointer_v1 test protocol, with a frame callback after each successful dispatcher tick. A simulated Escape signal cancels the adapter. It asserts exactly two sender axis packets and two frames, with no later tick.
- A disposable Nix derivation builds the sender from source. The workflow reuses the existing V5C-B native XDG/subsurface observer, default-hard-OFF SwayPX Nix compositor and locked renderer dependencies.
- A separate shell fixture launches a private headless SwayPX on a disposable GitHub runner, identifies mapped observer bounds via Sway IPC, runs the engine sender, and requires two positive vertical wl_pointer.axis messages and two matching wl_pointer.frame receipts, with no unrelated button or horizontal axis.
- The guarded /dev/udmabuf CI-only ACL procedure restores exact previous permissions. No operator-local changes.

## VERIFY BOUNDARY

This verifies that actual numerical autoscroll calculations can cross a native Wayland protocol transport into a mapped SwayPX client in a private headless session.

It **does not** prove:
- Physical libinput origin (the identity facts are synthetic).
- Compositor-internal trust production (the sender is a separate CI-only process).
- That SwayPX's compiled hard-OFF autoscroll runtime emitted any axis (it did not).
- Compositor-based pointer ownership or object-lifetime epochs.
- Production Escape binding, anchor indicator, Brave/Kitty/Zellij behavior.

An externally emitted virtual axis cannot satisfy the physical-device authority required for live autoscroll.

## SECURITY / OWNERSHIP

Company Map #126 remains open/paused and owns overlapping cursor.c, seat.c and meson.build paths. This work does not edit those or the parent #146 branch. Runtime PA_AUTOSCROLL_LIVE_ACTIVATION 0 remains unchanged; the separate production physical, output-generation and Scroll Lock gates remain closed. No IPC endpoint, uinput device, real input handler, compositor configuration, live mouse input or restart is added.

## CHECKLIST

- [x] Recheck Syntagma, current master/PR #146 and Company #126 owner state
- [x] Add independent V1/V2/V3 -> virtual Wayland transport sender
- [x] Reuse existing native observer and headless default-OFF SwayPX
- [x] Add axis/frame receipt and simulated Escape cancellation assertions
- [ ] Exact-head native Wayland Nix CI PASS
- [ ] Independent security/source review
- [ ] Shared cursor ownership approval and genuine physical-origin producer
- [ ] Compositor-owned epoch lifetime/scene/output trust
- [ ] Actual compositor-generated wl_pointer.axis with separate headless-only gating
- [ ] Explicit operator acceptance before merge/deployment

## NEXT

After V5D verification, request a scoped shared-cursor owner handshake. The owner must build a compositor-owned libinput physical source classifier and mint/invalidate real device/seat/target/output epochs. Then perform separate disposable native-compositor scroll delivery testing. Keep production OFF until accepted.

## RECOVERY

Close the stacked V5D draft to abandon it without touching the user desktop; V5C-A/B #146 remains independently reviewable. Stacked PRs marked merged do not automatically mean their content reached master.
