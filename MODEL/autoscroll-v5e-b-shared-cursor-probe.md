# V5E-B // Shared Cursor Source Candidate Probe (Production Hard OFF)

**Operator authorization:** November 19, 2026. The Company Map owner is paused; the operator explicitly approved this narrow autoscroll shared-input change **provided it is isolated and discoverable by the Company owner**. This does not transfer the Map Room or authorize merging/production.

**Patient:** kudokudo1/Post-Apollo-SwayPx
**Autoscroll Room:** Mouse & Keyboard · Universal Autoscroll
**Other Room:** Company native MapLibre in-widget scene, PR #126
**Staff:** External Hospital-compatible DOC; no live Hospital attachment claimed.
**Base:** V5E-A source-only origin policy PR #148, itself based on current master.

## IMPLEMENTATION

A **single limited shared-input block** in sway/input/cursor.c now:
- Runs only after BOTH Weather and Company pairing guards have first refusal.
- Reads backend and libinput scroll mode **only if** the optional autoscroll runtime exists; factory returns NULL in ordinary SwayPX because PA_AUTOSCROLL_LIVE_ACTIVATION is 0.
- Calls the existing private autoscroll function with an explicit, non-authorizing origin facts snapshot.
- Falls through unchanged to normal cursor/button dispatch if the runtime returns false.

Other new files supply a read-only wlroots/libinput candidate probe. The policy requires the pointer backend to be libinput-backed, a valid handle and readable method, and rejects button-scroll methods (the native Sway Scroll Lock collision).

**DO NOT CLAIM PHYSICAL AUTHENTICATION:** A kernel uinput device may appear libinput-backed. The presence of the backend and a readable libinput method is not a credential. Actual hardware provenance, device lifetime epochs, seat/target/output generation and operator-consent policy need separate trusted compositor proofs. The runtime still has physical_verified=false, native_scroll_lock_active=true, layout_generation=0, and production compile gate 0.

### Exact B-owned changes
- **Shared collision:** sway/input/cursor.c, sway/meson.build.
- **Autoscroll-only:** include/sway/input/post_apollo_autoscroll_source.h; sway/input/post_apollo_autoscroll_source.c; include/sway/input/post_apollo_autoscroll_runtime.h; sway/input/post_apollo_autoscroll_runtime.c; V5E-B tests/workflow/docs.
- **Unchanged:** sway/input/seat.c; seatop_default.c; Company scene/input/popup geometry policies; other workers' PR branches; operator Fedora runtime.

## HOW TO USE
Review only. Normal Windows-style autoscroll does not turn on. No user machine write.

## VERIFY
- Source tests verify ordering, candidate-only semantics, and hard-OFF gates.
- GitHub runner compiles real SwayPX with the new native libinput probe and no deployment.
- Test scope cannot validate real hardware or actual compositor axis delivery.

## WATCH OUT FOR / OWNER HANDOFF
Company Map #126 has its own cursor.c and meson.build changes. When its paused owner resumes, they must explicitly review this PR and reconcile the shared lines while keeping Weather + Company pairing, Company scene hit/previous-focus containment, then optional autoscroll, then ordinary dispatch. The branch must NOT be blindly merged or rebased into #126. A comment on PR #126 links this PR as a durable handoff; it is not proof of notification delivery.

## NEXT
Independently review current B source and CI. Follow with a distinct CI-only compositor physical-origin/udev/lifetime issuer and native event tests only after authorization of that larger scope. Do not flip the live activation macro.

## DECISIONS
Draft, unmerged, source-only non-authorizing shared seam; no ownership transfer and no local user desktop change.
