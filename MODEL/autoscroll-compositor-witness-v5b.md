# SwayPX // Universal Autoscroll V5B: Pointer Witness Policy + Mock Delivery

**State:** Draft/test-only, not a runtime activation. No physical pointer capture, no live Wayland axis delivery, no compositor configuration change. \`PA_AUTOSCROLL_LIVE_ACTIVATION 0\` remains unchanged.

## Hospital continuity

- **Patient:** \`kudokudo1/Post-Apollo-SwayPx\`
- **Room:** Native Universal Autoscroll / V5B Pointer Witness Test Contract
- **Staff:** external Hospital-compatible DOC; neither live registration nor shared-cursor ownership verified.
- **Assignment:** advance V5B test evidence using separately owned files, keeping input handlers and Company Map #126 untouched.
- **Base:** recovered V5A device+seat paired-release fix (PR #144 was merged into a nested feature branch). This new branch targets current \`master\` and carries that fix forward. Verify the exact PR head and compare before merge.
- **Ownership:** Map #126 is paused but remains open/draft. \`cursor.c\`, \`seat.c\` and \`meson.build\` are not changed in V5B.

## IMPLEMENTATION

This stage adds a **portable, deny-first policy**, not an authority issuer.

- \`post_apollo_autoscroll_witness.{h,c}\` checks a prospective compositor-issued event's origin, seat/device/target/output identity, mapped and directly hit-tested target, enabled output, layout generation, <=80 ms observation age, session lock, native Sway Scroll Lock conflict, Stage Director paired-input consumption, and pointer constraint.
- \`pa_autoscroll_witness_same_session\` compares a checked start witness with a newly observed witness, rejecting changes to device, seat, target, output or layout generation. It does not require the old start event to remain fresh throughout a long scroll session.
- A standalone C test **simulates** an allowed click, relative motion, and a mock axis/frame sink. It refuses virtual events and stale, inconsistent or locked observations; aborts on target or output generation changes; and refuses to claim a frame if the mock sink rejects the axis.
- V5A paired-release identity repair is included because #144 did not place it on \`master\`.
- CI compiles the portable policy and the existing V1/V2/V3 logic with strict warnings and runs mock-only regression tests. A source check verifies no witness policy call or activation path was added to \`cursor.c\`, the production runtime or Meson.

## Authority boundary

**These booleans and integer IDs are NOT self-authenticating evidence.** A caller in the production compositor must establish them from original trusted libinput events, pointer hit-testing, seat/output lifecycle and verified Scroll Lock state. A fabricated positive struct in a test proves only policy behavior, not true hardware origin. The fixture deliberately simulates physical-pointer provenance.

No new uinput/evdev access, Wayland global, IPC, public cursor-coordinate endpoint, keyboard hook, new permission, or runnable enable flag is supplied.

## VERIFY

- Exact-head GitHub autoscroll C test and grammar workflow results: **pending**.
- Existing V4 \`PA_AUTOSCROLL_LIVE_ACTIVATION 0\` must remain in force.
- These tests do **not** launch SwayPX, deliver real \`wl_pointer.axis\` events, demonstrate user-visible scroll, verify escape, or draw an overlay.
- A successful native compositor build is separate from a runtime witness or delivery proof.

## WATCH OUT FOR

Physical provenance and ownership cannot be inferred from \`struct wlr_input_device *\` identity alone. Device or seat pointer values may be reused after teardown unless lifecycle validity is separately proven. Likewise, a target-node ID is not an immutable proof of live surface ownership without lifetime checks. Real output-generation witness and native Sway Scroll Lock state require owner-reviewed source wiring.

Never remove the three hard-off runtime protections just to make this fixture pass.

## NEXT

1. Validate this draft and restore the V5A fix to mainline only after operator/owner acceptance.
2. Obtain shared-input owner acceptance for a disposable headless SwayPX authority producer.
3. Implement a separately guarded, test-only compositor ingress that derives actual libinput/scene/output/Scroll Lock witnesses (not client assertions), then test with native headless Wayland clients.
4. Add deliberate Escape cancellation, input-transparent anchor, and Brave/Kitty/Zellij acceptance; keep production OFF until authorized.

## DECISIONS

Draft, source/test-only, unmerged, no local desktop changes. Company Map #126 ownership unchanged. Rollback by closing draft or reverting exact changes if independently merged.
