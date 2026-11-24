# 🏥💜 V5F-G3F // Two-tier input trust boundary, source-only proposal

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Universal Autoscroll · Input Trust  
STAFF ROLE // DOC, external Hospital-compatible, not live-registered  
PARENT // G3E draft PR #175, source head `37c962fb47b8ac9952057f8eda8478b01344c021`; independently reviewed by Nurse 2, scoped PASS  
OWNERSHIP // Autoscroll test/model branch only; Company Map #126 retains its shared input ownership; TimeW #176 is separate  
PHASE // G3F proposed security policy MODEL, not an approved or deployed input grant  
PRODUCTION // HARD OFF · No workstation change · No actual scrolling

## IMPLEMENTATION

**PROPOSED two levels, plus the default OFF state.** The operator agreed to explore a two-tier policy, but this document is not a production permission decision or a claim of genuine hardware proof. Its C model lives **only** in `tests/autoscroll/`. No normal SwayPX production source or Meson references it. It does not create a settings UI, enrollment credential, broker, live session, or connected Hospital assignment. It has no physical verifier and all authorization functions always return false.

| Mode | Proposed meaning | Diagnostic output now | What is explicitly NOT proven |
| --- | --- | --- | --- |
| OFF, unselected, invalid | No Autoscroll trust requested | DENIED | Everything |
| OS-managed | Operator accepts the LOCAL OS/seat/libinput trust boundary for convenience | OS_SOURCE_CANDIDATE_ONLY, if all source observations claimed | Hardware physical authenticity or user intent; a privileged kernel uinput device can spoof input |
| Enrolled-device | Operator would require an additional independently authenticated device/session trust broker | ENROLLMENT_BROKER_REQUIRED, if native source candidate | No trusted issuer, anti-replay, enrollment, challenge response, or hardware attestation exists |

Neither non-OFF result is an activation. In particular, **OS-managed source candidacy is not hardware attestation**. A device that passed libinput/udev filters may still be kernel-synthetic (uinput, privileged emulation, virtual HID). Explicitly known synthetic or Wayland virtual devices are denied even by this more permissive proposed candidate filter. Unknown sophisticated spoofing remains possible; OS-managed would accept this risk only after a separate operator decision about the OS trust boundary.

The test-only evaluator demands as NECESSARY observations: actual native compositor event callback, current nonzero event time, middle-button press, pointer class, not Wayland-protocol virtual, not already known synthetic, libinput backend/handle, coherent candidate udev topology, known native scroll method without conflicting button-down scrolling, current device/seat lifetimes, event device attached to same seat, and Stage Director Weather/Company first refusal. All are untrusted **caller-supplied model bits** in this harness. Even a fully forged true set can only return the named **diagnostic**. A successful model status does not create a compositor-minted credential.

The enrolled lane deliberately ignores `claimed_device_enrollment`, `claimed_matching_serial` and `claimed_signed_witness`. An application-selected serial, vendor/product ID, USB port/path, sysfs node, or unsourced signature bool cannot authenticate a device. A future **trusted broker** requires a separately reviewed source of proof, scoped device/session identity, protected key custody, one-use event liveness or sequence, revocation, immutable provenance binding and safe handling of reconnection. Even a privileged OS broker that relies on evdev identity alone is stronger at access control, but **not hardware attestation** against an attacker allowed to present synthetic HID. Ordinary mice generally cannot sign cryptographic challenges. Strongly hardware-attested mode may need special devices or a different trust design. Do not promise plug-and-play authenticated hardware enrollment for generic mice.

## HOW TO USE

There is NO shipped setting or user-facing toggle in G3F. For future design, the operator may choose OFF / OS-managed / enrolled-device on an Autoscroll settings screen, but the location, labels, persistence and default-on behavior are **PROPOSED**, not existing UI. Default OFF persists.

For this milestone, review G3F's draft PR and its CI evidence. No workstation command, device enrollment, local settings change or user action required.

## VERIFY

Source-only C/Python test scope:
- Compile the isolated policy model and adversarial harness with C11 `-Wall -Wextra -Werror`. No wlroots, wayland, udev, or compositor object is linked.
- Check **524,288** combinations of the 16 necessary source facts, four mode values and selected/unselected, enforcing exact diagnostic outputs and zero permission to consume middle or emit axis in EVERY case.
- Specifically reject virtual pointers despite owner epochs and valid scene, recognized kernel synthetic paths, native button-scroll conflicts, missing seat or event owner, and forgeries of enrollment, serial and claimed signature.
- Recheck original `sway/input/post_apollo_autoscroll_runtime.c` hard-off macro, physical false, native Scroll Lock hard deny, bridge origin UNKNOWN, unverified layout, and no G3F object reachable from production.
- Enforce additive test/docs/workflow file set against G3E and no overlapping changed path with paused Company Map #126.
- Require exact-head GitHub grammar + G3F tests and an independent Nurse 2 review.

These tests validate **inert model behavior**, not the security of a future issuer. G3E remains the actual native negative virtual-event test. G3F does not reuse forged positive G3C witness as any kind of physical attestation.

## WATCH OUT FOR

A real input source can be created by kernel uinput, passed through libinput, and possibly made to resemble a physical bus topology. OS-managed candidacy must never be described as `trusted_physical_pointer`. There is currently **no trusted issuer**, and the enrolled lane never upgrades to a positive result. Hardware tokens cannot be reconstructed from source-provided bools. A future implementation must explicitly decide who owns trust, what privileges local processes have, how a real click is bound atomically to the current seat, output, view and event, what happens with multiple devices and a virtual keyboard changing Scroll Lock, and when to revoke.

Before any actual scrolling, the separate independent guards must verify: selected OS/enrolled policy from trusted configuration; native provenance policy; genuine compositor-owned owner epochs; exact active direct mapped XDG window; output-layout generation; fresh Scroll Lock state (UNKNOWN denies); pointer constraints/session lock; Stage Director first refusal; exact check-before-axis and frame; inactivity/Escape/source loss cancellations; and an operator-approved display/indicator. G3F alone proves none of these production paths.

TimeW #176's transaction callbacks and C5 consent are **different** security dependencies and do not authorize Autoscroll. Do not move Workspaces or change compositor production transaction files as part of this Room. Company Map #126's production cursor/seat seam still needs coordination before any merge of the parent Autoscroll branches.

## CHECKLIST

- [x] Re-read Syntagma authority, Hospital role/report and current Autoscroll G3E source heads.
- [x] Separate two-tier non-actuating C model and exhaustive adverse input matrix.
- [ ] GitHub exact-head CI + independent Nurse 2 G3F review.
- [ ] Operator ratifies OS-managed exposure to kernel synthetic inputs versus stronger enrolled model; choose threat assumptions.
- [ ] Design and independently verify actual trusted compositor-owned source producer (if chosen), without boolean provenance claims.
- [ ] Privileged broker/enrollment identity protocol, if enrolled mode selected, including liveness/revocation.
- [ ] Layout/Scroll Lock/scene atomic check, native scroll delivery, cancel & UI and desktop acceptance.

## NEXT

Run CI. Independently review source-only behavior, including fake input flags yielding only candidate output. Prepare a **decision record marked PROPOSED** for operator ratification, then implement the minimal authorized lane without changing the production hard-OFF switch until the missing independent safeguards and release gates are complete.

## DECISIONS

*Proposal only*: model OFF + OS-managed source-candidate + enrolled-device broker-required lanes as separate concepts. **Not decided**: exact approved threat model, ability to enroll generic hardware, privilege boundary, production activation, UI, owner merge or device attestation. Production autoscroll HARD OFF; TimeW Restore HOLD; Company Map owner preserved.
