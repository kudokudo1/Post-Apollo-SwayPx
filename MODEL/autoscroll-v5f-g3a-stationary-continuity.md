# 🏥💜 V5F-G3A // Stationary-scrolling continuity contract (detached; HARD OFF)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Middle-Click Autoscroll  
STAFF ROLE // External Hospital-compatible DOC; live registration NOT verified  
BASE // V5F-G2 [PR #163](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/163) at `13d9e99f98b211cd24c193380840c210fa9a9cab`  
PARALLEL OWNER // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126), paused, unchanged and NOT reassigned  
SCOPE // Bounded GitHub source/tests/docs only. No merge, workstation change or production authorizations.

## IMPLEMENTATION

The current V5C portable `pa_autoscroll_epoch_tick()` requires a strictly newer `observed_at_ms` from an input event **on every 16ms timer callback**. That is reasonable against *replayed events*, but a stationary mouse creates no new event while the user still wants ongoing Windows-style scrolling. Increasing an event timestamp on each timer would fabricate physical activity.

This G3A introduces a **detached, NOT Meson-linked** alternative policy under:
- `include/sway/input/post_apollo_autoscroll_continuity.h`
- `sway/input/post_apollo_autoscroll_continuity.c`

The detached `pa_autoscroll_continuity_begin()` deliberately REUSES real V5C `pa_autoscroll_epoch_begin()` for the start-event age and internal field checks. A future compositor-owned native producer must separately authenticate that original input event. **The code does not do that and all-positive caller-supplied facts remain forgeable.**

A timer then receives a separate freshly reread **scene/owner snapshot**, not a new input witness. `pa_autoscroll_continuity_tick()` compares exact four originally issued owner lifetimes (device, seat, mapped target, enabled output), their IDs, unchanged layout generation and independent current scene state. It cancels if the current sample is missing, older than 48ms, future-dated, or if any native lock, pointer constraint, Stage Director pairing, target mapping, output, hit-test or layout validation is negative. Tick time must be increasing with a gap no greater than 250ms; a long pause cancels rather than burst scrolling. Cancellation zeroes state and requires an entirely new start.

The **actual** start event's `observed_at_ms` is frozen: no fresh events are invented. A valid session can continue while the cursor is stationary for many ticks. No maximum session duration is imposed by this detached reference policy, but the current scene and timing must be revalidated before every emission.

This is an **unintegrated candidate**, not production authorization. A dishonest caller could fabricate either the start witness or repeated positive scene snapshots. A trustworthy protected compositor-private producer and atomic read/check/send ordering are still missing.

## HOW TO USE

Review the G3A draft PR and GitHub CI. No operator desktop steps, no package installation, no Mouse settings changes and no compositor restart. Production remains `PA_AUTOSCROLL_LIVE_ACTIVATION 0`.

## VERIFY

The exact-head workflow must:
1. Compare strictly with G2 head and permit only the two new detached C/H files, isolated C matrix, Python source safety test, workflow, and this note; no existing source, Meson, Company or Weather edits.
2. Assert production runtime and guard bridge remain **hard OFF** and do not reference the candidate.
3. Compile the REAL existing `post_apollo_autoscroll_epoch.c` and `post_apollo_autoscroll_witness.c` together with the new detached candidate, using `-std=c11 -Wall -Wextra -Werror -pedantic`.
4. Prove the legacy tick cancels an unchanged input event; prove the detached scene policy permits **1000 × 16ms ticks with ONE unchanged input event**, conditioned on newly sampled matching scene data.
5. Reject independent device, seat, target, output, ID, layout, lock, Scroll Lock, Stage Director, pointer constraint, mapping, stale/future scene, timeout, duplicate timer and missing-scene changes. Every refusal clears its guard, and no tick rearms automatically.
6. Preserve the exact original checkout and forbid output/input/Wayland/axis side effects.

No positive native input or actual compositor is launched by G3A; prior C1/C2/G2 real negative native receipts remain separate evidence.

## WATCH OUT FOR

**Physical identity and trust remain unresolved.** The producer cannot assert `PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT` solely because libinput or sysfs looks physical. G1 demonstrated a forged positive C witness satisfies the detached portable policy. G3A's positive synthetic values are the same sort of test-only input, not hardware credentials.

**Trust model alternatives, explicitly PROPOSED rather than accepted:**
- *OS-trusted compositor input provenance:* regard events from tightly bound, native compositor-side libinput ingress as OS-trusted input while excluding exposed Wayland virtual pointers, known uinput, mismatched kernel event paths and delegated handlers. This is not an anti-spoofing guarantee against compromised kernel/userspace or emulated USB HID.
- *Stronger device-specific attestation/enrollment:* require independently managed device enrollment/provenance, if the actual hardware and OS supply it. General HID/USB mice do not uniformly have cryptographic attestation.

Neither alternative has operator approval as a canonical positive-authority model. Source-visible event flags and `scene.captured_at_ms` are NOT trustworthy by themselves: these must be owned and generated inside a reviewed native module, with no IPC/QML/test hook controlling them. Respect Weather/Company first refusal, real Sway Scroll Lock, screen lock and per-axis ordering; reject owner drift between the last check and dispatch.

The max scene age (48ms) and scheduler gap (250ms) are provisional *safety bounds*, not an adopted final UX decision. Verify in disposable compositor scheduling before production, including sleep/resume, output loss and shared-owner race windows.

## CHECKLIST

- [x] Recheck G2 exact head, master, current PRs, Syntagma and Company Map ownership.
- [x] Detached continuity C/H policy with strict reuse of original start guard.
- [x] Reproduced exact legacy stationary conflict plus 1000-tick alternative matrix.
- [x] No build/runtime/Company integration and explicit trust-model undecided.
- [ ] Exact-head workflow PASS and independent review.
- [ ] Approved OS/hardware trust threat model and compositor-owned real input producer.
- [ ] Native trusted scene capture, source/session lock, Scroll Lock and same-owner evidence with atomic admission-to-axis path.
- [ ] Actual native stationary session, real app receiver, Escape/indicator and final operator desktop acceptance.
- [ ] Parent/Company shared-seam reconciliation and independent merge decision.

## NEXT

Run exact-head CI and record receipts. Seek independent security review of the detached timer-policy limits and physical-origin trust choices before production integration. G3B could independently implement a fail-closed *negative-only* native scene revalidation test against a disposable compositor while keeping `PA_AUTOSCROLL_LIVE_ACTIVATION 0`. No positive origin or live-axis dispatch should be introduced without a separately accepted native trust model.

## DECISIONS

**G3A = DETACHED CONTINUITY CANDIDATE + NEGATIVE SAFETY TESTS.** No positive physical authorization, production timer, runtime link, Company ownership transfer, merge, installation or final operator acceptance. Pending threat-model choice remains **PROPOSED**.
