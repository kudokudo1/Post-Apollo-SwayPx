# SwayPX // V5E-D1 Actual Compositor Device Lifetime Wiring (HARD OFF)

**STATE:** draft, source-integrated pointer device lifecycle only. NOT all four object lifetimes, NOT physical authentication, NOT native autoscroll authorization, NOT merged/deployed.

## HOSPITAL

- PATIENT // kudokudo1/Post-Apollo-SwayPx
- ROOM // Mouse & Keyboard / Universal Autoscroll
- STAFF ROLE // External Hospital-compatible DOC; no verified live Hospital registration
- ASSIGNMENT // Continue the bounded autoscroll work, preserving original Company Map #126 ownership and owner handoff
- BASE // stacked on [V5E-C #151](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/151) exact tested head a36d4ebd6aefd79bf7a9b7dc3e7332a55f5fabf5
- PERMISSION // Operator previously allowed a clearly split, narrow shared-input change while Company Map's owner was paused. This D1 package deliberately avoids the Company's cursor/seat/window/output handlers.

## IMPLEMENTATION // Device lifetime hooks

V5E-C added a detached monotonic issuer. D1 now integrates that issuer at **real owner-controlled SwayPX input manager callbacks**.

- The input manager owns one zero-initialized issuer, initialized **once**, before it subscribes to backend input events.
- Each input-device wrapper owns a lifetime slot; real pointers receive a new, non-repeating token when SwayPX registers the wlroots device.
- Protocol-created virtual pointers also receive their *own* lifetime token for tracking, but the read-only accessor returns ZERO for virtual devices. A lifetime token must never be confused with a valid physical input source.
- The existing input device destroy callback revokes the slot BEFORE seat device removal and wrapper free. A reused wrapper address cannot carry a prior live token.
- An internal read-only accessor returns an active token only for a nonvirtual pointer wrapper that is still live; it does not claim that a kernel/libinput backend is guaranteed real physical hardware. A separate udev filter and final provenance issuer still have to be checked.
- The portable lifetime implementation is newly compiled into real SwayPX through Meson.

**The actual compositor source is now wired to the device lifetime issuer.** The exact-head full SwayPX build plus source-order tests and detached unit fixtures are the intended evidence. CI in this package does NOT yet run a dedicated headless event trace that proves real device create/destroy callbacks were dynamically observed; it checks the actual binding points and linked binary, not an event trace.

## OWNERSHIP / DISTINCT PACKAGES

**V5E-A #148:** policy only; no shared input source.
**V5E-B #149:** read-only libinput candidate probe, own previously authorized cursor/Meson touch.
**V5E-C #151:** read-only udev negative topology and detached object-lifetime issuer; only a Meson overlap.
**V5E-D1 (this PR):** changes input-manager.c, input-manager.h, Meson, CI and tests. This child does not change cursor.c, seat.c, container.c or output.c.

The Map worker owns [Company #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126). Before any merge, the returning owner should review any actual shared-path overlap. A PR comment on #126 will carry an exact-head recovery pointer. A durable GitHub comment is not proof of live notification delivery.

## HOW TO USE
GitHub review and CI only. No CPU++ Settings toggle, user-desktop writes or service restarts.

## VERIFY
- Source-only callback ordering and virtual-pointer refusal: pending exact-head CI
- Portable C11 issue/revoke/address reuse/overflow: pending
- Real Nix-built SwayPX with lifetime source linked: pending
- Real callback event logs: not yet
- Seat, target, output create/destroy event wiring: NOT IN THIS PACKAGE
- Genuine physical hardware origin + native Scroll Lock and output scene binding: NOT IN THIS PACKAGE
- Native compositor-generated autoscroll: still HARD OFF

## WATCH OUT FOR
An input device lifetime token is not physical authentication. A compromised kernel or synthetic hardware can still pass topology; a virtual-pointer protocol must remain refused even when it has a valid lifetime token. An expired, removed or untracked pointer must produce token zero.

The source implementation has a finite UINT64_MAX token capacity and refuses overflow. Input devices without a minted token keep functioning normally, but can never be considered for future privileged autoscroll.

Runtime PA_AUTOSCROLL_LIVE_ACTIVATION 0, physical_verified=false, native_scroll_lock_active=true and layout_generation=0 remain unchanged.

## NEXT // V5E-D2+

D2: seat lifecycle owner integration with distinct tokens and cancellation on seat teardown.
D3: view/target node and output lifetime create/unmap/destroy integration, including layout generation invalidation.
Then securely join the V5C guard/issuer and actual physical origin without trusting user-controlled paths; run headless and eventually hardware acceptance. Preserve independent review and explicit operator authorization for merge/live actuation.

## DECISIONS
Draft/unmerged. No Company Room reassignment. No local workstation edits, PR merges, SwayPX restart or live activation.
