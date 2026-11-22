# 🏥💜 V5F-G3D // Real compositor scene and XKB Scroll Lock diagnostic audit (NEGATIVE, HARD OFF)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC; live Hospital registration unverified  
BASE // G3C [PR #167](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/167), exact `dd2eb2b5ebfa31fe9627fc85a52807f52bf34c81`  
INDEPENDENT REVIEW // Nurse 2 source+native negative verdict, [review 6013508146](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/167#pullrequestreview-6013508146), scoped PASS  
OWNERSHIP // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) still separately owned, paused and unmerged  
AUTHORITY // GitHub CI-only test/docs. No merge, operator workstation change, positive physical authorization, original compositor source mutation or autoscroll activation.

## IMPLEMENTATION

G3C showed all FOUR genuine native object lifetime tokens and negative cancellation in ONE disposable compositor, but used synthetic scene/Scroll Lock state. G3D separately observes a real native `node_at_coords` scene hit and output layout at a cursor position, and asks the actual compositor seat keyboard for the XKB Scroll Lock **LED** status. Crucially, this is still read-only diagnostic data, not permission to consume physical input.

G3D adds exactly EIGHT new test, workflow and documentation paths. Its standalone C diagnostic probe is in `tests/autoscroll/`, **never referenced by original `sway/meson.build`**. A temporary GitHub runner checkout copies SwayPX to `RUNNER_TEMP`; only there, the `seat seat0 cursor set` IPC code is instrumented to run the probe **after its real Sway cursor warp + scene rebase**, and only there, the test probe is added to an instrumented Meson build.

The probe samples real compositor state synchronously:
- `node_at_coords` hit: accepts a direct actual mapped XDG view surface, rejects ambiguous popup/subsurface/layer, dirty/instruction-pending/destroying target or missing hit.
- `node_get_output` and `wlr_output_layout_output_at` must refer to the same real, enabled output at cursor coordinates; disabled/fallback/destroying/dirty/instruction-pending output refuses.
- Owner-issued target/output epochs must be nonzero, and session lock/pointer constraints/synthetic touch-pointer modes must be absent.
- Seat's `wlr_seat_get_keyboard` must exist, have an actual keymap/XKB state, and expose `XKB_LED_NAME_SCROLL`; otherwise Scroll Lock is `UNKNOWN`, **never assumed OFF**.
- Scroll Lock `ON`/ `OFF` comes from `xkb_state_led_index_is_active` of that native keyboard. No physical-input origin or real multi-keyboard global Scroll Lock policy is established.

A disposable Wayland virtual-keyboard client supplies an XKB keymap and toggles Scroll Lock through the native Wayland keyboard protocol. Because the default XKB map exposes the Scroll Lock LED but does not necessarily make `<SCLK>` change its locked modifier, the CI client adds **`modifier_map Mod3 { <SCLK> };` only to its own disposable test keymap**. This allows real native XKB LED OFF → ON → OFF observations without changing the workstation's keymap or native production keyboard rules. The client sends **no pointer button/motion/axis** and never requests autoscroll. It lets us observe the compositor's **actual** LED state OFF, ON, OFF and UNKNOWN after detaching the test keyboard. Keyboard is a **VIRTUAL TEST DEVICE**. Its LED state is native but can be controlled by an untrusted software client. It is not a physical HID proof.

The native CI sequence also moves the actual headless seat cursor onto the center of a real Foot XDG window via normal Sway IPC, then off that view and finally onto the old coordinates after unmap. That confirms real target/layout presence and fail-closed miss transitions. No scrolling runtime or owner guard is ever started, and no axis/frame is emitted **by the G3D probe**.

## HOW TO USE

Review the draft G3D PR, dedicated Actions workflow and native receipt. Do not run locally. The test's runner path/identity and `/dev/udmabuf` ACL wrapper prevents operation on a normal workstation and restores any temporary runner-only permissions.

## VERIFY

Require at exact head:
1. Strict 8-file additive diff vs G3C and zero overlap with paused Company Map #126's 26 changed files.
2. Production runtime still `PA_AUTOSCROLL_LIVE_ACTIVATION 0`, physical witness false, Scroll Lock hard-deny true and origin UNKNOWN; no G3D probe linked into production.
3. CI source tests verify only temporary copied compositor `seat/cursor.c` and `sway/meson.build` are instrumented, with callback AFTER real warp/rebase, no source checkout dirty.
4. Real pinned Nix SwayPX build, Foot XDG scene and native virtual-keyboard test client.
5. Real native diagnostic receipts: no keyboard=UNKNOWN; real XDG direct hit+actual layout match; virtual keyboard LED initially OFF, then ON, then OFF; no target at off-window coordinates; no target at old coordinates after XDG unmap; after keyboard destruction Scroll Lock=UNKNOWN.
6. No physical/HID claim, no application axis, no production activation; GitHub runner ACL restored.
7. 2/2 exact-head workflow results; independent Nurse 2 review of G3D separately required.

## WATCH OUT FOR

**This is a diagnostic, not a trusted producer or compositor-atomic authorization.** A native XKB Scroll Lock LED can reflect an untrusted virtual keyboard. It may not represent the actual Scroll Lock policy across multiple keyboards, seats, configuration overrides or compositor pairing modes. This test does not establish a source-specific physical-operator lock witness.

Likewise, a direct XDG hit and same-output check at one instant do **not** establish an independently minted layout generation. The G3D C probe reports `layout_generation=UNVERIFIED`. It does not combine four owner epochs with active hit/LED into a guard or send axis/frame; the check-to-axis race remains OPEN.

The G3D cursor-command hook is **only in the disposable runner copy**. It is NOT an additional public IPC capability, production code addition or UI feature. The native Sway `seat cursor set` command already exists; this test only logs its resulting observed scene in CI. It does not turn the normal cursor command into a trust issuer.

A desktop with multiple keyboards or an attached virtual keyboard cannot treat `XKB_LED_NAME_SCROLL` as trusted physical-state authority merely because the bytes were read from native XKB. Additional multi-device conflict/unknown policy must be reviewed. Operator approval for OS-trusted libinput vs stronger hardware evidence also remains OPEN.

Parent autoscroll branches still have shared cursor/seat/Meson ownership seams with paused Company Map #126, and this G3D zero-overlap child does NOT resolve those integration conflicts.

## CHECKLIST

- [x] Reconcile independent Nurse 2 G3C scoped PASS at exact commit.
- [x] Read native source for hit-test, output-layout, XKB, IPC seat cursor command and hard-OFF policy.
- [x] Eight test-only files with native read-only scene/LED observer and virtual-keyboard client.
- [ ] Exact-head native CI PASS; no production or Company file changed.
- [ ] Independent G3D security review of source, client and test-only native outputs.
- [ ] Decide trusted physical-origin semantics and real Scroll Lock policy, with operator.
- [ ] Real layout generation, Stage Director pairing, four-owner + native scene and exact check-to-axis dispatch ordering.
- [ ] Active middle-click feature, Escape, indicator, software UX and desktop acceptance.
- [ ] Company seam reconciliation and explicit merge authority.

## NEXT

Run exact-head CI and resolve source-only test failures in G3D. Request independent G3D review. If native XKB LED fails to reflect virtual protocol toggles, report FAIL without falsely claiming real state; correct the isolated test client or scope, not production runtime.

After G3D, design a **private, fail-closed trusted native observation interface** for real scene/lock/origin with explicit operator threat-model choice and independent review BEFORE any active axis integration.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION HARD OFF · NO PHYSICAL ORIGIN · REAL NATIVE DIAGNOSTIC ONLY · COMPANY MAP OWNER PRESERVED · FINAL ACCEPTANCE PENDING.
