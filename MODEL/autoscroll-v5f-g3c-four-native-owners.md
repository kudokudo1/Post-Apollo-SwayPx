# 🏥💜 V5F-G3C // One disposable compositor, four genuine native owner epochs (negative only)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC, not verified as live Hospital staff  
BASE // G3B [PR #165](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/165), exact `17db838bc80d4d44dc0cf75d04c188bfb3f3b231`  
OWNER // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) separately owned and paused  
AUTHORITY // Bounded GitHub-only CI/test/docs. No merge, workstation modification, positive physical-input trust or live scrolling enablement.

## IMPLEMENTATION

G2 showed genuine virtual device/seat lifetimes in one compositor; G3B separately showed target/output epochs and cancellation. G3C puts **all four genuine native lifetime domains in a single disposable compositor session**. The newly compiled test-only probe samples:
- Device: `pa_autoscroll_lifetime_epoch(&device->autoscroll_lifetime, PA_LIFETIME_DEVICE)`, after a real wlroots virtual pointer was registered.
- Seat: `seat_autoscroll_lifetime_epoch(seat)`, after a real transient seat protocol resource was created.
- Target: `container_autoscroll_target_lifetime_epoch(target)`, after an actual Foot XDG-shell window mapped.
- Output: `output_autoscroll_lifetime_epoch(output)`, after the real headless output was enabled.

**Critical boundary:** The same virtual pointer's **physical eligibility** via `input_manager_autoscroll_pointer_lifetime_epoch(device)` must remain **ZERO**, despite a positive *raw lifetime token*. Raw owner object identity is not physical provenance. The test deliberately supplies a **FORGED physical start-event witness and synthetic layout, hit-test, lock/pairing and scene fields** to call the detached G3A policy. These fixtures exist only in the test probe in the temporary GitHub build. No one may promote a test-only forged positive record into a real native authorization producer.

The real Wayland client creates a transient seat and virtual pointer through pinned protocol XML, then controls only their destruction. **It never sends button, motion, or axis events.** Real Foot XDG clients and native output config commands provide target/output lifecycle changes. Five independent synthetic guard starts are tested in the SAME compositor process, each with four native owner-issued nonzero distinct epoch tokens:
1. Native output reconfiguration (epoch rotates): G3A guard must cancel; other three epochs unchanged.
2. Real window unmap (target epoch becomes zero): guard must cancel; other three unchanged.
3. Real virtual-pointer destruction (device epoch becomes zero): guard must cancel; the pointer remains physically ineligible throughout.
4. Real transient-seat destruction **before its virtual pointer** (seat epoch becomes zero): guard must cancel; other three unchanged. No physical/Wayland input emitted.
5. Real output disable (output epoch becomes zero): guard must cancel; other three unchanged.

The G3C probe is compiled only in the CI-copied SwayPX under `RUNNER_TEMP`. Original source, native production runtime, Meson, user input and workstation are never changed. Exactly eight new test, documentation and workflow paths are added; the existing source callbacks are altered only in that private copied build. No configured autoscroll engine, timers, axis or frame dispatch.

## HOW TO USE

Review the draft PR and GitHub Actions logs. No local actions. Never run these runner-only scripts on an operator workstation. The CI wrapper gates GitHub-specific identities, paths, root status and `/dev/udmabuf` access; it restores temporary ACL changes even on failure.

## VERIFY

Exact-head GitHub CI must establish:
1. Strict eight-file diff against G3B, no Company Map #126 changed paths, no production/CMake/Meson changes and `PA_AUTOSCROLL_LIVE_ACTIVATION 0` with native UNKNOWN/physical=false/Scroll Lock=true gates.
2. Source overlay tests verify copied C1+C2 owner callbacks and G3C probes execute after genuine lifetime binding/revocation.
3. Real Nix-built disposable SwayPX, actual pinned Wayland virtual-pointer/transient-seat client and Foot XDG window app.
4. For each baseline, log four simultaneously observed **real native epoch tokens**, all nonzero and distinct, and confirm physical eligibility remains zero.
5. Five real negative guard cancellations exactly at native output, target, device and seat lifecycle callbacks, with only the named owner token changed. No replay admission, no fake hardware attribution and zero axis/frame emission.
6. Clean original checkout, temporary runner ACL restored, exact-head 2/2 workflows and independent Nurse security review (still separate).

## WATCH OUT FOR

Four native object epochs being available together is not **atomic authorization**. They are inspected through compositor callbacks but this rehearsal does not prove that the IDs, target hit-test, mapped window-to-output relationship, real native layout generation, pointer constraints, session lock, Scroll Lock, Stage Director first refusal, event provenance and the future check-to-Wayland-axis transaction are all trustworthy and synchronized.

**Never claim physical attestation:** the Wayland virtual pointer's hardware eligibility remains ZERO. The positive witness in the test is deliberately forged. G1 showed that the portable C policy accepts caller-supplied positive fields; only a separately reviewed private native producer could protect this boundary. An OS-trusted libinput event may be distinguishable from Wayland virtual injection but is not cryptographic proof of user-operated hardware. Stronger hardware enrollment is a separate optional threat model. No canonical choice has been accepted.

Four-owner cancellation and actual live active scroll/indicator/Escape are different requirements. No scroll mode is enabled. Existing parent branches still overlap Company Map #126 at old shared cursor/seat/Meson seams; no merge or Room takeover is authorized.

## CHECKLIST

- [x] Check G3B exact head, current G3A and Company Map #126 ownership before new branch.
- [x] G3C CI-only observer for four actual compositor object lifetimes.
- [x] Separate real transient Wayland client controls native device and seat teardown.
- [x] Five real owner-revocation negative scenarios scoped; no scroll events.
- [ ] Exact-head CI PASS and durable handoff to Company owner.
- [ ] Independent Nurse 2 review and operator-accepted physical/OS-origin threat model.
- [ ] Trusted native start producer, real scene/hit-test/Scroll Lock/constraint witnesses.
- [ ] Atomic/serialized check-to-axis and stationary active scrolling with Escape/indicator.
- [ ] Mainline reconciliation, end-user Fedora desktop acceptance and explicit merge authorization.

## NEXT

Run exact-head native CI; fix test-only failures in this branch while preserving original source. Report exact logs for each of five owner changes and virtual physical-eligibility=0. Seek independent review before ANY positive start event or axis integration. Next safe work could focus on a compositor-produced **negative-only** scene/Scroll Lock evidence audit and application button receipt, not a live autoscroll toggle.

## DECISIONS

**G3C = NEGATIVE FOUR-OWNER NATIVE LIFETIME REHEARSAL, NOT TRUSTED INPUT OR ACTIVE SCROLL.**  
DRAFT · UNMERGED · HARD OFF · NO PRODUCTION/LOCAL CHANGES · NO COMPANY MAP OWNERSHIP TRANSFER · FINAL OPERATOR ACCEPTANCE PENDING.
