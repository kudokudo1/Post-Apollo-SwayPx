# V5F-G1 // Trusted Physical Input: Boundary Audit Before Authorization

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC, live registration not verified  
BASE // V5F-C2 [PR #161](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/161) · `bf5c2a3f0a0754ef7d8939d02105b251cf3109c7`  
OWNER // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126), paused and NOT reassigned  
AUTHORITY // Explicit operator "continue" for bounded GitHub source/test work. No PR merge, local host changes or production activation.

## IMPLEMENTATION

Adds only four **test/docs/workflow** files. No compositor C, headers, Sway cursor/seat, Meson, output, Weather or Company Map files change.

The new C adversarial harness compiles the actual version-pinned source of:
- `sway/input/post_apollo_autoscroll_origin_policy.c`
- `sway/input/post_apollo_autoscroll_topology.c`
- `sway/input/post_apollo_autoscroll_witness.c`
- `sway/input/post_apollo_autoscroll_epoch.c`

It verifies that bus-looking sysfs topology and libinput facts produce **only a candidate**, and that virtual, mismatched event paths and unknown/native button-scroll modes are denied. It then tests the V5C guard's negative evidence, independently refused conditions (Scroll Lock, layout, Stage Director pairing, active pointer constraint, locked session, revoked output, stale/future event) and replay/owner/layout mismatch cancellation.

**Key adversarial finding:** a C caller that fabricates all the right fields, including `PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT`, can satisfy the **portable** epoch guard. This is not a bug in the portable validator, but it proves the *critical native trust boundary*: the compositor must never accept proof from raw serialized structs, IPC, QML, a virtual pointer, libinput candidacy, a udev path, a timer or an unapproved test hook. F-B's production native bridge is deliberately UNKNOWN, layout-unverified, independent lock-gate denied and ultimately hard-false. No trusted positive producer currently exists.

## VERIFY

The GitHub workflow must enforce an exact four-file diff against the pinned F-C2 branch, run source safety tests, compile the actual four C modules with `-std=c11 -Wall -Wextra -Werror -pedantic`, run the adversarial cases and show the original checkout remains clean. This test does **not** boot a compositor, inspect physical host hardware or exercise active Wayland scroll axis/frame output; the earlier C1/C2 disposable native callbacks remain separate verified evidence.

**Expected output:** `PA_G1_REAL_C_ORIGIN_CANDIDATE_NOT_CREDENTIAL_PASS`, `PA_G1_REAL_C_POLICY_NEGATIVE_MATRIX_PASS`, `PA_G1_FORGED_POSITIVE_ACCEPTED_BY_PORTABLE_POLICY_ONLY_NO_REAL_HARDWARE_ATTESTATION_PASS`, `PA_G1_REPLAY_OWNER_LAYOUT_TIMER_CANCELLATION_PASS`.

The counterintuitive "forged positive accepted by portable policy" label is an intentional *negative architectural discovery*, not production success. A pure policy function that accepts a caller's boolean facts cannot itself prove where an input event came from. Only a separately designed and reviewed privileged producer can establish the missing evidence.

## HOW TO USE

Review the draft PR and GitHub tests. No production setting, terminal commands, desktop reboot, package installation or source merge is needed.

## WATCH OUT FOR

The Linux kernel's evdev/uinput, userspace virtual input, software-emulated HID, USB-device identity and libinput backend paths are different layers. A libinput device is not necessarily a physical human-operated mouse. A sysfs path resembling USB or PCI is a useful negative filter but cannot establish physical origin against all synthetic/compromised devices.

**Additional UX/trust conflict discovered:** the present `pa_autoscroll_epoch_tick()` requires a strictly newer input `observed_at_ms` at **every 16 ms timer tick**. Real Windows-style autoscroll must keep scrolling while the pointer stays still at an offset. Fabricating a fresh mouse event each timer tick would be incorrect and dangerous; enforcing the current rule will cancel stationary scrolling. Before a positive active implementation, independently review separate *start-event authenticity*, *event-driven owner revocation* and *timer-tick safety* semantics, including bounded inactivity policy. This G1 package does **not** relax the current guard or mint freshness.

**Unresolved design choice** for a future operator decision:
- Trust a compositor-internal libinput-origin event as an **OS-trusted pointer event** (not a cryptographic guarantee of real hardware), with explicit protection against Wayland virtual pointers and known uinput devices, documented threat limits; or
- Require stronger device-specific/OS-managed hardware attestation or operator enrollment. General USB/HID devices do not universally offer such attestation, so blanket "physically verified" cannot be promised merely from device metadata.

No one may silently promote the less strict assumption as canonical hardware proof. Positive authority must be reviewed against the chosen threat model. In both cases preserve Weather/Company first refusal, session lock, native Scroll Lock, layout/hit testing and owner epoch invalidation.

## CHECKLIST

- [x] Source-of-truth current branches, Syntagma and owner preflight
- [x] C-level adversarial libinput candidate vs physical-proof boundary
- [x] Forged positive witness vulnerability exposure and timer freshness gap
- [x] Four-file test/docs/workflow-only change; no Company Map overlap
- [ ] Exact-head workflows PASS and durable owner handoff
- [ ] Independent review of physical-input threat model and evidence producer
- [ ] Event-driven lifetime safety without fake timer freshness
- [ ] Positive native origin/scene/Scroll Lock, controlled active scroll test and UI

## NEXT

Finish this source-only adversarial verification, report the precise missing attestation dependency and request independent security review. A subsequent G2 may add CI-only **real virtual middle-click negative** integration evidence without enabling autoscroll. A true positive producer remains a separate owner-reviewed design and implementation.

## DECISIONS

G1 = **AUDIT + NEGATIVE TESTS**, NOT PHYSICAL VERIFICATION.  
DRAFT · UNMERGED · PRODUCTION HARD OFF · NO LOCAL MACHINE WRITE · NO SHARED PRODUCTION SOURCE CHANGES · COMPANY MAP OWNER PRESERVED.
