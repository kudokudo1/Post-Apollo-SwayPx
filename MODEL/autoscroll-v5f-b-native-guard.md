# V5F-B // Pinned V5C Native Guard Bridge (Production HARD OFF)

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC, no verified live Hospital registration  
ASSIGNMENT // Reconcile the pinned V5C epoch policy into the D3/F-A feature branch and gate native runtime before middle-click or Wayland axis/frame dispatch  
BASE // V5F-A PR #158, verified f096acac4db63f1afd546ad11811aaa94907889e  
PIN // Master at preflight fb064cf3c82aa7b9e8d2375bb631773d5eb5a12f  
PERMISSION // Operator-approved bounded source changes; NOT a merge, local host write, production enable or Company Map Room takeover.

## IMPLEMENTATION

The original V5C portable epoch guard's C and header bytes are copied **without changing its policy logic** from the exact master commit. The V5C witness C/H already present on the feature branch had the same Git blobs as master. Both guard and witness files are linked to the actual SwayPX compositor together with a new private native guard bridge.

A bridge reads F-A's verified four-object snapshot synchronously from compositor-owned device/seat/window/output objects, constructs the four typed epoch values, and calls V5C begin or tick. The native runtime owns a separate guard per cursor and performs:
- On native middle-button press, after Weather/Company first refusal and the libinput origin candidate, **before** calling pa_scroll_dispatch_middle: attempt guarded begin. A refused begin cancels the engine and returns false without consuming the event.
- At native timer tick **before** pa_scroll_dispatch_tick and any Wayland axis/frame callback: attempt guard tick, fail closed and cancel on missing live object, invalid epoch or missing fresh evidence.
- During runtime cancellation/destruction: cancel both engine and guard. Axis/frame output paths also require an active guard.
- Any current native snapshot is only object continuity, **not** a real hardware credential. The bridge sets physical origin UNKNOWN, layout generation unverified/zero and native Scroll Lock active/unknown. A timer lacks an independent native pointer event timestamp, so the bridge never invents one from the timer clock. Consequently both native begin and tick are **deliberately nonadmissible**, even if someone later accidentally changes the engine enable path.

This is a genuine source-level guard connection with compile-time HARD OFF, not a live feature activation. The production runtime still allocates nothing and installs no timer when PA_AUTOSCROLL_LIVE_ACTIVATION is zero.

## OWNER BOUNDARIES

Company Map PR #126 remains open/paused, owning cursor.c, seat.c, seat.h, Meson and several Company focus and popup files. F-B production changes are confined to new guard bridge C/H, exact V5C epoch C/H, existing autoscroll runtime.c and a Meson source addition. F-B changes **no cursor, seat, Company scene, XDG, transaction, output or input-manager files**. The only new Company shared-file overlap is Meson. Note F-A parent already overlaps Meson; do not double-count the same shared file as a new Company subsystem implementation.

Changes to the older inherited workflow allowlists are made in this child only, with hard-OFF source guards and an explicit changed-file manifest on the dedicated F-B CI. The previous F-A owner snapshot stays read-only and unchanged.

## VERIFY

Dedicated F-B GitHub Actions:
- Pin exact V5C header/source and witness header/source blobs against `fb064cf3c82aa7b9e8d2375bb631773d5eb5a12f` (no unnoticed behavior drift).
- Assert the bridge never asserts PHYSICAL_LIBINPUT, never fabricates a layout generation, and native timer passes no synthetic new-event timestamp.
- Prove bridge begin precedes middle-dispatch and tick precedes axis-dispatch in actual SwayPX native code; cancellation erases both guard and engine.
- Re-run strict C11 V5C portable legacy and F-A independent four-domain revocation, changed-epoch/same-slot-memory/replay/Scroll Lock tests.
- Verify Company Weather/Company pairing first refusal and HARD OFF, run actual Nix full SwayPX binary build and inherited workflows.

These tests do NOT provide a positive physical-input witness or dynamic monitor/hotplug/window-teardown trace. An actual compositor event loop is compiled but does not activate autoscroll; no Wayland axis/frame was sent by this feature.

## WATCH OUT FOR

The staged feature tree and the master branch are still separate histories. Copying V5C's source into a child feature line is a **source-level reconciliation**, not a conflict-free merge of full feature/master histories. Independent reviewer and owner must resolve the whole feature branch against the current master and Company #126 before accepting an actual merge.

The returned tokens are monotonic only inside one issuer session, and do not prove pointer source trust, live scene focus, per-tick event freshness, cross-restart identity, or complete layout mutation coverage. This bridge is intentionally a hard negative guard. It cannot be switched on by merely changing a boolean and must NOT be treated as an authorized feature.

## HOW TO USE

Review the draft and exact-head GitHub CI. No UI toggle, Sway/Quickshell restart, permission change, local workstation modification or running-compositor feature change.

## CHECKLIST

- [x] Re-read Syntagma, latest master and feature branch heads, Company ownership
- [x] Integrate exact pinned V5C source and witness into real SwayPX build
- [x] Gate native middle-button and timer axis paths through owner-issued epochs
- [x] Forbid positive physical/layout/Scroll Lock witness and timer-faked freshness
- [x] Dedicated strict source, portable C and native build workflow
- [ ] Exact-head all relevant CI PASS
- [ ] Durable Company #126 owner handoff with exact overlap and head
- [ ] Independent security review and operator merge approval
- [ ] Dynamic native device/seat/target/output lifecycle receipts and trusted witness producer
- [ ] Verified physical origin, native Scroll Lock, scene/layout generation, Escape/indicator and actual desktop acceptance

## NEXT

V5F-C should prove actual object create/revoke and cancellation with disposable native compositor event traces, while keeping ALL positive live input gates off. Then independently verify the physical provenance/consent path and a new timestamp from each trusted input callback. Do not permit production activation until independent approval.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION OFF · NO LOCAL HOST WRITES · COMPANY MAP OWNER RETAINS PR #126.
