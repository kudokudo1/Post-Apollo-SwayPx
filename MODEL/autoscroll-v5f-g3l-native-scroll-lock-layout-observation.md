# 🏥💜 V5F-G3L // Actual XKB Scroll Lock and output-layout observations, denial intact

PATIENT // Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed native input safety  
STAFF ROLE // DOC · External Hospital-compatible; live registration not verified  
BASE // Reviewed G3K #186 exact head `84cde37ce4aea4176ef9a685827e52f2cc811936`; Nurse 2 scoped native PASS  
PRODUCTION // Autoscroll HARD OFF · TimeW Restore HOLD

## IMPLEMENTATION

G3K proved a real Linux kernel uinput mouse travels through libinput/wlroots/SwayPX, preserving native seatop while refusing a synthetic origin and after real target unmap. G3L extends its **disposable compositor COPY only** with a **separate real kernel-uinput keyboard** and a **real native output-layout IPC change**.

The kernel actor advertises one known-synthetic pointer and one distinct known-synthetic keyboard, both deliberately fake BUS_USB. Neither is physical hardware. The actual SwayPX input manager must register the second device as a keyboard. The new read-only native compositor observer is called from the actual SwayPX mouse middle-button cursor callback. It locates ONLY the test keyboard attached to the same seat and queries its **native XKB Scroll Lock LED via `xkb_state_led_name_is_active`**. Missing keyboard, multiple matching devices or missing XKB state yield `UNKNOWN/DENY`, not an inferred unlocked state.

The same native callback reads **`wlr_output_layout_get_box(root->output_layout,NULL)`** and records actual bounding rectangle. The runner proves baseline XKB LED OFF during the first two real middle-button pairs; then sends real `KEY_SCROLLLOCK` press/release into Linux via that separate keyboard, requires its native XKB LED to show ON during a third mouse pair, and moves the single active headless output through SwayPX IPC. A fourth real mouse pair must observe the changed native layout rectangle. The native output command and `get_outputs` record must independently confirm this change. The earlier G3K real XDG target unmap and kernel device deregistration remain required; the original guard begin/tick continues refusing ALL eight native mouse events.

**Key security distinction:** An actual XKB LED reading from one software-created keyboard is **NOT verified multi-keyboard global Scroll Lock policy**. The first native run registered the keyboard and received pointer events but observed LED OFF after the kernel Scroll Lock key transition; the missing mapping/update is a real unresolved condition, not an unlocked witness. G3L therefore requires the actual SwayPX keyboard callback and does not require or imply a positive LED ON state. A live layout bounding box is **NOT a monotonic generation token** and cannot prove every scene commit or exclude later changes. Both remain `UNKNOWN/DENY` in production. This experiment cannot mint trusted origin, lock-clear, layout generation, permission to consume middle clicks or permission to emit an axis. The native runtime is still compile-time HARD OFF.

## HOW TO USE

Inspect draft G3L GitHub Actions logs. No operator desktop control, workstation command, installation or activation. The test compositor, private seatd and both uinput software devices run only inside a strictly guarded disposable GitHub-hosted VM. The actor and runner explicitly clean up kernel devices and restore temporary /dev/udmabuf ACL.

## VERIFY

- Exactly eight new MODEL/CI/test paths. No modifications to tracked production runtime, cursor, keyboard, seat, output code, normal Meson, Company Map or TimeW.
- Actual SwayPX native libinput keyboard registration, before/after true `KEY_SCROLLLOCK` and `EV_SYN` kernel events, a real SwayPX KEY_SCROLLLOCK press/release callback, with XKB LED read before and after; the default keymap may leave the LED unchanged, which remains UNKNOWN/DENY.
- Actual output `get_outputs` geometry changed following SwayPX `output ... pos` IPC, with the fourth middle-pair callback reading the same changed layout x/y/width/height.
- Inherited real G3K XDG target unmap and native device removal; all eight real middle button events still dispatched normally and guard begin/tick refused, zero axes/frames.
- Model and source invariants plus inherited regression suites. CI fail if kernel keyboard/LED/output evidence unavailable; no fallback/mock acceptance.
- Independent Nurse 2 exact-head review before merge; operator workstation acceptance separate.


## LIMITATION RECONCILIATION

The first successful native compositor build exposed an important real-world discrepancy: a kernel `KEY_SCROLLLOCK` press/release can be emitted by a registered software keyboard without the tested keyboard's XKB Scroll Lock LED changing. This is **NOT a passing Scroll Lock activation or proof that Scroll Lock is clear**. G3L explicitly checks the native keyboard callback and both observed LED values, then requires continued DENY regardless. A future separately reviewed milestone must prove live native lock semantics for all seat keyboards, keymaps, groups and hotplug, or decline to activate.

## WATCH OUT FOR

G3L is a **native state observation**, not a complete independent authority for scrolling. The XKB LED of the specific synthetic keyboard is not system-wide multi-device lock state; user keyboards may disagree or be hot-plugged, keyboard groups can alter semantics, and Scroll Lock can be remapped. A simple output-layout rectangle is not a protected generation counter and can suffer ABA/reconfiguration races. A real kernel uinput source is not physical HID. Need further independent current-state and freshness contracts before any positive admission.

Company Map #126 owns shared production input seams; no G3L checked-in code touches them. TimeW Restore HOLD and Sign-In HOLD remain separate. G3K #186 and its parents remain open draft and unmerged despite scoped reviews.

## CHECKLIST

- [x] Reconcile G3K scoped Nurse 2 PASS and preserve production hard-off.
- [x] Author native keyboard/layout observer, two-uinput-device actor, four-phase native trace and isolated CI instrumentation.
- [ ] Prove exact-head XKB LED/layout change with real SwayPX callbacks.
- [ ] Independent Nurse 2 review.
- [ ] Real full multi-keyboard Scroll Lock policy, complete layout generation and hotplug ABA prevention.
- [ ] Esc cancellation, atomic check-before-axis, actual app scroll receipt, owner seam integration and operator activation approval.

## NEXT

Run exact-head GitHub CI; correct only disposable fixture mistakes, never weaken native source/guard refusal. After scoped review, specify protected **multi-keyboard lock + full layout generation** semantics and test deny-on-conflict/ABA in a separate milestone.

## DECISIONS

OS-managed first remains the operator direction, enrolled-device verification optional. G3L is CI-only read-only negative authorization plus live XKB/layout observations. Autoscroll HARD OFF; TimeW Restore HOLD. No merges or workstation changes.
