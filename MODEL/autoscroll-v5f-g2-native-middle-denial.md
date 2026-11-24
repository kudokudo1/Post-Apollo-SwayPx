# 🏥💜 V5F-G2 // Native virtual middle-click denial (CI only)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC; live Hospital registration not verified  
BASE // V5F-G1 [PR #162](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/162) · `0ab46ae3303bd357ea7ccb747567ea23bab62235`  
OWNER // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) remains separate, paused, not reassigned  
SCOPE // Operator-authorized bounded continuation; GitHub CI/test/docs only; no merge, local host or production activation.

## IMPLEMENTATION

This is a **native negative ingress test**. The G2 client is a genuine Wayland client using the same pinned `ext_transient_seat_v1` and `zwlr_virtual_pointer_v1` protocol sources that passed C2. Unlike C2, G2 sends one real **virtual middle-button press and release** over the compositor's private Wayland socket. It sends no movement or axis events.

The runner reuses C2's actual device/seat lifecycle callback overlay and instruments the **real SwayPX `cursor.c` in a disposable copied checkout only**. It observes:

1. A real virtual pointer bound to a real transient seat is created. The device receives an owner lifetime token, but its physical eligibility remains **zero**.
2. The middle press and release both enter the normal native `handle_pointer_button` callback.
3. Weather and Company retain first refusal, and neither unarmed pairing consumes the test events.
4. The autoscroll runtime is **NULL** under `PA_AUTOSCROLL_LIVE_ACTIVATION 0`.
5. Both button transitions reach `dispatch_cursor_button`, call the actual `seatop_button` dispatch routine, and return.
6. The virtual pointer and transient seat are destroyed through real callbacks and their owner epochs are revoked.

The test does **not** create a focused application window or verify delivery of a `wl_pointer.button` event to a client surface. Its precise deliverability claim is **native compositor/seatop dispatch preserved**, not application receipt. It does not test real hardware, a positive guard, physical scroll, continuous timer behavior or overlays.

Only six new test/workflow/docs files are added. No original production C/header/Meson/source, Company Map, Weather, input config or operator workstation files change.

## HOW TO USE

Review the draft PR and GitHub workflow. No user desktop action or activation is necessary. All native execution is restricted to disposable GitHub CI. The harness enforces `GITHUB_ACTIONS=true`, `CI=true`, fixed runner temp path, nonroot identity and explicit G2 and C2 test gates. Temporary runner-only `/dev/udmabuf` ACL changes must be restored on exit, including failure.

## VERIFY

Before claiming a native PASS, require:

- Exact six-file diff relative to G1, hard-OFF source gates, no Company changed paths.
- Source tests covering the actual cursor callback, Weather/Company first refusal order, runtime NULL and seatop dispatch.
- Compiled SwayPX from disposable overlay and a Wayland client using pinned protocol XML and Nix dependencies.
- Runtime logs for **both** press and release: ingress, no pairing consumption, seatop entry and return; runtime NULL and device physical eligibility 0.
- Real protocol destroy and matching native owner revocation.
- Zero local workstation mutation, clean source checkout, restored temporary device ACL.
- Exact-head GitHub Actions success; independent Nurse security review and operator acceptance remain separate.

## WATCH OUT FOR

The test intentionally injects **virtual input**, so any positive autoscroll activation would be a failure. The absence of the native runtime under hard-OFF is expected, not a substitute for independently proven real-hardware authorization.

This test does not validate that an application receives the click. An end-to-end client `wl_pointer.button` receipt requires a focused test surface and is a separate, tighter acceptance step.

Physical input provenance remains unresolved (libinput candidate != hardware identity). G1 also identified that `pa_autoscroll_epoch_tick` requires new observed input on each 16ms timer tick, conflicting with stationary continuous scrolling. G2 does not change that design or invent event freshness.

Parent feature branches still contain compositor input/seat/Meson overlaps with paused Company Map #126. Do not merge them into master without independent integration reconciliation. Company retains its original ownership.

## CHECKLIST

- [x] Rechecked canonical Syntagma, G1 exact head and separate Company owner.
- [x] New six-file CI-only source/test package on child branch.
- [ ] G2 exact-head native GitHub workflow PASS.
- [ ] Independent security review and owner report.
- [ ] Real physical-origin/native trusted producer model.
- [ ] Event-driven owner revocation plus stationary timer continuation design.
- [ ] Authenticated synchronized four-owner positive session, native Scroll Lock and scene.
- [ ] Indicator/Escape, mainline reconciliation, operator-approved Fedora desktop acceptance.

## NEXT

Complete and verify G2 GitHub native workflow. If passed, record exact logs and state the narrower result precisely: a virtual press/release is not swallowed by disabled autoscroll, and both reach normal native seatop. Then request independent review of the G1 physical/OS-trust model before authorizing any positive event producer. Consider a focused-surface client receipt test as a distinct evidence stage, not a hidden expansion of G2.

## DECISIONS

G2 IS A **NEGATIVE NATIVE DISPATCH TEST**, NOT PHYSICAL PROVENANCE OR POSITIVE ACTIVATION.  
DRAFT · UNMERGED · HARD OFF · NO LOCAL HOST CHANGES · NO COMPANY OWNERSHIP TRANSFER · NO FINAL OPERATOR ACCEPTANCE.
