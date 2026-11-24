# 🏥💜 V5F-G3I // Actual Linux libinput middle event, negative source proof

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed event evidence  
STAFF ROLE // DOC · External Hospital-compatible, no live registration verified  
BASE // G3H PR #181 exact head 0e2896eb5b92819bcec90f61544120a7aebf838c  
REVIEW // G3H independent Nurse 2 exact-head scoped native PASS #6021128847  
OPERATOR DIRECTION // OS-managed Linux mouse first, enrolled-device verification optional later  
PRODUCTION // HARD OFF · No local workstation changes · No application scroll

## IMPLEMENTATION

The previous stages are different:
- G3G: real Linux uinput pseudo-mouse reported by real libinput; no actual button events sent; kernel virtual sysfs source refused.
- G3H: real compositor callback with Wayland virtual-pointer press/release over an actual XDG window; OS-managed diagnostic refused; normal seatop path intact.
- G3I (this step): an actual **Linux uinput EV_KEY BTN_MIDDLE press and release**, each followed by EV_SYN SYN_REPORT, delivered through a real libinput path context and returned as two `LIBINPUT_EVENT_POINTER_BUTTON` events, **without** a compositor, Wayland virtual-pointer transport or forged libinput events.

G3I's disposable GitHub-hosted Ubuntu runner creates exactly one real uinput pseudo-mouse claiming fake BUS_USB, with real /dev/input/eventN and udev sysfs identity. It opens that event node in a libinput path context, matches libinput's device and udev node to the original kernel identity, and asks **unchanged** Autoscroll topology and origin policy to classify it. The virtual kernel path must be denied. It then sends exactly one middle press and release to the own uinput fd and verifies both appear via `libinput_dispatch`/`libinput_get_event` with matching real device identity, button code, state and nonzero monotonic libinput timestamps. No wheel, pointer motion, application target, compositor source handler or scroll output is allowed.

**Critical distinction:** kernel uinput is a real **OS input pathway** but it is NOT genuine physical hardware. Its fake USB bus declaration must not authenticate it. G3I does not prove acceptance of a genuine physical mouse, and does not prove an actual `wlr_input_device_is_libinput` compositor callback. Those require a separately verified native compositor / real device setup. If that future fixture is unavailable, the test must report unverified rather than claim physical hardware was accepted.

This G3I test compiles the original production `sway/input/post_apollo_autoscroll_topology.c` and `sway/input/post_apollo_autoscroll_origin_policy.c` as-is. No test-only custom topology implementation is allowed. Both OS-managed G3F actuation functions remain false and normal installed SwayPX stays compile-time HARD OFF.

## HOW TO USE

No desktop settings or instructions. Inspect the GitHub draft's exact-head Actions logs only. The GitHub-hosted runner must explicitly opt into the CI kernel test and confirm it's running in the ephemeral runner before requesting root privileges for its one temporary `/dev/uinput` device. If `uinput` isn't present, CI may load it inside its own disposable runner; no persistent `/dev` permissions, host workstation files, real desktop mouse or local services are changed. A test failure is a failure, not a passing skipped fixture. Cleanup destroys the device with `UI_DEV_DESTROY`, including on verification failures.

## VERIFY

- Exactly FOUR additive paths: native C, Python contract tests, MODEL and GitHub workflow. No SwayPX production cursor/seat/runtime/Meson/TimeW/Company source file changed.
- Real kernel device, fake BUS_USB, real libinput path/udev identity match, topology DENIED.
- Native `LIBINPUT_EVENT_POINTER_BUTTON` press and release with BTN_MIDDLE, correct order, matching libinput device, real nonzero timestamps.
- No libinput axis, no wheel, no app input receipt and no compositor injection. Unmodified production classification refuses virtual source before and after events.
- Source regression tests from G3I/G3H/G3G/G3F/G3E/G1 and exact-head CI; no changes to original checkout.
- Independent Nurse 2 exact-head review required before any merge or broader integration.

## WATCH OUT FOR

A privileged software-created kernel mouse can masquerade as USB while still being synthetic. A virtual sysfs node is strong **negative** evidence, but absence of a virtual path does NOT authenticate a physical mouse. G3I does not use the SwayPX compositor or yield a positive OS-managed candidate. It does not consume app events or prove scroll delivery. Approval of OS-managed first is a development decision, not a runtime permission.

Future positive admission requires actual compositor-owned source and seat evidence, current directly mapped target and output generation, native Scroll Lock and Stage Director priority, cancellation and final check-before-axis. Company Map #126 owns the shared production pointer seam. The separate TimeW SwayPX #182 chronology checker corrections are its owner's job.

## CHECKLIST

- [x] Reconcile and verify G3H independent scoped PASS; preserve its source and Company Map boundaries.
- [x] Write real kernel-to-libinput BTN_MIDDLE event observer and deny-only source path.
- [ ] Verify real 2-transition native events and cleanup in exact-head disposable GitHub CI.
- [ ] Independent Nurse 2 G3I review.
- [ ] Future compositor libinput backend event test using an actually OS-managed device, without hardware-authentication claims.
- [ ] Native Scroll Lock/layout/cancel/dispatch and desktop acceptance under operator control.

## NEXT

Run exact-head CI. No synthetic/mock fallback. If real libinput events cannot be observed, report failure and correct the isolated CI fixture, not production source.

## DECISIONS

OS-managed first accepted, enrolled-device optional later. G3I is an **actual Linux libinput event** negative-source milestone, not positive physical trust. Autoscroll production HARD OFF; TimeW Restore HOLD; no merge or workstation action.
