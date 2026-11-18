# SwayPX // V5C-A Object-Lifetime Admission Readiness (source-only)

**STATUS:** Draft source-only auxiliary task, NO physical input, NO compositor hook, NO delivered `wl_pointer.axis`, NO native activation. Production stays hard OFF.

## HOSPITAL CONTINUITY

- **Patient:** `kudokudo1/Post-Apollo-SwayPx`.
- **Room:** Mouse & Keyboard / Universal Autoscroll V5C, limited early object-lifetime continuity assistance.
- **Worker role:** External Hospital-compatible DOC; operator requested additional assistance while the primary PX Room lifecycle assignment remains distinct. No live Hospital registration, no assumed reassignment of the Mouse & Keyboard or paused Company cursor owner.
- **Base:** SwayPX `master` at `3c650ce311ea3f37bb8c0421d60dcfa7cb5f0b12`, following PR #143 and PR #145 actual mainline merges. PR #144's device+seat release protection arrived in `master` through #145.
- **Ownership:** Company Map #126 remains an open draft and owns overlapping `cursor.c`, `seat.c`, `meson.build`. This change only adds independently named epoch source/header/test, documentation and separate CI.
- **Scope:** Harden a *future* compositor-owned witness admission against recycled object addresses. This is not the V5C live input integration and does not replace its owner.

## IMPLEMENTATION

V5B `pa_autoscroll_witness_same_session()` matches device, seat, output, target and layout generation, but still trusts numeric IDs. A libinput device, seat, scene target or output may be destroyed and re-created with an identical pointer address or integer ID. Such reuse needs a separate owner-minted **object-lifetime epoch**.

This branch introduces `pa_autoscroll_epoch_begin/tick/cancel`, a portable nonproduction guard:

1. Start only with an admissible V5B witness and four nonzero object-lifetime epochs (device, seat, target, output).
2. Compare all four epochs at every later tick, in addition to existing V5B origin, target, output, layout and source checks.
3. Require a **new event observation timestamp** after the previous tick, without requiring the first observation to stay fresh for the entire session.
4. On one mismatch, stale/replayed tick, lock, virtual source or missing evidence, clear the guard and deny future ticks until a new begin.
5. No functions mint epochs, fabricate physical-source status, call wlroots, open devices, send axis/frame, or enable any runtime feature.

The source-only unit fixtures provide *synthetic* initial event, elapsed ticks, fake pointer-address reuse with changed epochs, replay, locked seat, virtual event and scene/output change. A successful unit test only validates behavior assuming an independently trusted producer.

## V5C-B // Actual headless virtual-input refusal (new CI-only fixture)

The isolated `tests/autoscroll/ci_headless_virtual_denial.sh` boots the **real default-hard-OFF SwayPX compositor** on a private headless Wayland socket on a disposable GitHub Linux runner. Two separately compiled, native Wayland clients use the pinned repository's Wayland and wlroots protocols:

- A mapped XDG + wl_subsurface observer records actual `wl_pointer.enter`, middle button press/release and any `wl_pointer.axis`.
- A virtual-pointer sender uses **`zwlr_virtual_pointer_v1`** to inject only BTN_MIDDLE press, relative motion and release. This is explicitly a *virtual* device, not libinput hardware.
- The test must see normal BTN_MIDDLE press **and** release in the target client, with **zero** unsolicited axis packets. The target coordinate comes from the compositor's actual Sway IPC tree, not arbitrary client-asserted geometry.
- The harness enforces `CI=true`, `GITHUB_ACTIONS=true`, a private `RUNNER_TEMP` runtime directory, a locked Nix SwayPX+Mesa build, timed cleanup and explicit refusal outside CI. There is no new production event hook, runtime enable flag, virtual-input authority or host deployment.
- The new workflow reports the exact outcome and still runs with `PA_AUTOSCROLL_LIVE_ACTIVATION 0`; the later **genuine physical-libinput** witness and authorized real `wl_pointer.axis/frame` integration remain open.

This is a **negative headless integration result**, not evidence that autoscroll works. If a virtual pointer ever starts an autoscroll or the client receives unsolicited axes, the test must fail. Real application + real physical input testing is a separate, owner-approved milestone.

## PHYSICAL EVENT AND HEADLESS LIMITS

A headless compositor with virtual pointer input does **not** establish the physical-libinput source demanded by V5B. A test that fabricates `origin=PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT` is a policy mock, not a genuine hardware ingress witness.

The future owner-approved compositor producer must:

- Derive source identity from the actual kernel/libinput device tied to the button event, never from QML/IPC assertions or pointer address alone.
- Mint and invalidate nonzero device, seat, target and output *lifetime* epochs at owner-controlled create/bind/destroy, with no reuse after teardown in the same runtime.
- Validate current surface hit, mapped target, output enabled/scene layout generation, pointer constraint, active seat, native Scroll Lock and Stage Director input ownership before any press is consumed.
- Verify device/seat ownership and a fresh hit+epoch sample at each scroll tick. If evidence disappears, cancel, do not silently send to the new target.
- Test actual `wl_pointer.axis` and frame delivery using a **separately approved, isolated** compositor test ingress and an observing client. This cannot be treated as proof of physical hardware when driven by a virtual headless device. Hardware provenance needs separate installed/physical test evidence.
- Keep `PA_AUTOSCROLL_LIVE_ACTIVATION 0` in production, and never bypass it with a normal environment variable or UI flag.

## HOW TO USE

No desktop commands, configuration changes or login attempts. The new separate GitHub workflow compiles only the portable epoch source with warnings-as-errors and asserts that no shipping compositor handler, runtime gate or Meson registration changed. Real input remains untouched.

## VERIFY

- Pure-C epoch guard is expected to deny stale/replayed event timestamps and changed owner lifetimes.
- Existing V5B witness and V5A paired-release fix on `master` remain unchanged.
- New workflow exact-head results and full compositor/headless regressions must be checked separately. No claim of genuine libinput or axis delivery is possible from these unit tests.

## WATCH OUT FOR

A positive witness structure, even with nonzero epochs, can be forged by an untrusted source. A valid event requires an independently authenticated compositor owner to supply its fields. Epochs are useful only if minted and retired by that owner.

The optional auxiliary test logic must **not** be linked to production or used to enable the runtime before cursor/input ownership coordination and separate operator-approved activation. The standing Social/SwayPX startup recovery and PX Room lifecycle workstreams must not be modified by this task.

## CHECKLIST

- [x] Confirm #143 and #145 actually delivered mainline, #144 nested, runtime hard OFF.
- [x] Identify open Company #126 overlapping input ownership.
- [x] Add test-only lifetime epoch contract and independent CI.
- [x] Add CI-only native virtual middle-click and observer fixture without shared cursor edits (exact-head outcome pending).
- [ ] Verify exact-head epoch and existing compositor regression checks.
- [ ] Coordinate with the Mouse/Keyboard primary and Company cursor owners before production handler changes.
- [ ] Derive genuine physical ingress/scene/output evidence in isolated compositor test.
- [ ] Verify Wayland client axis/frame and safe cancel under isolated test mode.
- [ ] Verify real hardware input and user-visible Brave/Kitty/Zellij behavior under explicit approval.

## NEXT

Hand this branch to the Mouse/Keyboard V5C owner as a source-only auxiliary readiness fixture. Ask the Company shared-cursor owner to approve a single, staged test-only source-of-truth ingress seam, then build compositor-derived event and epoch production in a disposable headless session. Do not claim headless virtual input proves physical origin. The primary PX owner may return to the Hospital protected-writer work independently.

## DECISIONS

Draft review only. No merge, local host install, real mouse capture, enable switch, compositor reload or changed Company cursor handlers. Rollback by closing this draft without merging.
