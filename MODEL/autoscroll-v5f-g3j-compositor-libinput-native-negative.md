# 🏥💜 V5F-G3J // Real compositor-native OS libinput input, synthetic source denied

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed native input  
STAFF // DOC · External Hospital-compatible, no live Hospital registration verified  
BASE // G3I PR #183 exact head `156eee505c4d3477db27e1ffb6856d3829a56b98` · independently Nurse 2 scoped PASS #6022458720  
DIRECTION // Operator accepted OS-managed first, enrolled-device optional later  
PRODUCTION // Autoscroll HARD OFF · TimeW Restore HOLD

## IMPLEMENTATION

G3G verified real kernel uinput source denial through libinput, G3H verified a real SwayPX compositor callback for a Wayland virtual pointer, and G3I verified actual kernel uinput middle events through standalone libinput. **G3J joins those separate paths inside an actual disposable SwayPX compositor**.

A one-use GitHub-hosted runner creates a real Linux uinput pseudo-mouse with a deliberately fake BUS_USB and a known synthetic name. It must have an actual `/dev/input/eventN` and udev ID_INPUT_MOUSE tag. The test runs an isolated **headless + libinput wlroots multi-backend** SwayPX copy using a private seatd session. Its native input manager must register the real kernel-backed device as a pointer. The real compositor `wlr_pointer_button_event` callback must receive exactly one BTN_MIDDLE press and release from **that** libinput device, not a Wayland virtual pointer or a test-only invocation.

The CI-only C observer inside the native callback verifies `wlr_input_device_is_libinput(device)`, the original libinput handle, device name, actual kernel udev path, virtual sysfs, input-manager registration, real device and seat epochs, current four-owner snapshot, Stage Director first-refusal facts and unmodified Autoscroll source classification. Original native G3H observer continues invoking the G3F **non-actuating** OS-managed diagnostic. The test insists this **real Linux OS-managed path is still synthetic**, so Autoscroll source candidate is DENIED. The original G2 native trace must show ordinary seatop receiving both button transitions; there is no consumption and no axis/frame dispatch.

This is a native `wlr_input_device_is_libinput` compositor observation, **NOT physical hardware attestation** or a positive physical mouse. The kernel device is deliberately software-created. Nonvirtual sysfs paths cannot authenticate hardware either. No future policy should silently convert OS-managed source candidacy or source-device epochs into a physical credential. Stronger device enrollment remains optional future work.

All checked-in production runtime/source, Meson, TimeW and Company Map files remain unmodified. Only a **disposable CI copy** of the compositor includes G3J observer; production runtime is still compile-time HARD OFF and null on startup. No direct input event on the user's workstation.

## HOW TO USE

Read the new G3J draft PR and its exact-head GitHub Actions logs. There is no desktop control, installation, service or local command.

The runner must prove `GITHUB_ACTIONS=true`, `RUNNER_ENVIRONMENT=github-hosted`, known runner temp and non-root invoking user. Only disposable runner CI may use root to create one kernel uinput source and start private seatd for a native SwayPX libinput backend. System seatd sockets must not preexist or be overwritten. A temporary runner /dev/udmabuf ACL change, if required, is restored. Actor closes/destroys its kernel device. Never run on user workstation.

If uinput, device udev tags, private seatd, native libinput backend, compositor input registration, XDG target or actual native callbacks are unavailable, **FAIL and report the blocker**. No mocked log fallback, no aliasing a Wayland virtual device to libinput, no timeout-as-success.

## VERIFY

- Exactly eight new CI/test/MODEL paths, zero production source edits and zero overlapping paths with paused Company Map #126.
- Actual udev-registered kernel test mouse with fake BUS_USB, native SwayPX input manager pointer, `wlr_input_device_is_libinput` true, actual libinput handle and sysfs /virtual/ evidence.
- Two real press/release callbacks from the same known synthetic libinput device, native G3H model DENIED, source candidate false and original seatop receives both transitions. No axis, guard authorization or event swallowing.
- Actual mapped XDG Foot target and output/scene observations must be reported accurately; no fabricated positive owner snapshots.
- Device teardown, seatd cleanup and runner /dev/udmabuf ACL restoration; source checkout clean.
- G3J source guards and G3I/G3H/G3G/G3F/G3E safety regressions plus exact-head grammar/native workflows.
- Independent Nurse 2 exact-head review. CI success alone does not authorize merge or workstation installation.

## WATCH OUT FOR

CI's real OS-input object is still **uinput software**, not a physical HID. This proves only SwayPX handles a real Linux libinput-backed device through wlroots and refuses this known kernel-virtual source. It does not prove positive hardware eligibility, consent, safe app scrolling, or a correct production source trust policy. Local privileged synthetic devices may disguise themselves as hardware. Production trust is intentionally inactive. Native Scroll Lock across keyboards, output-layout generation, Esc/cancel, final atomic check-before-axis, app receipt and user-facing autoscroll remain outstanding.

The TimeW SwayPX #182 corrected checker belongs to its own owner; Company Map #126 retains shared cursor/seat ownership. Parent Autoscroll branches require cross-owner reconciliation before merge.

## CHECKLIST

- [x] G3G #180, G3H #181 and G3I #183 independent scoped reviews verified; all three are unmerged drafts.
- [x] Author real kernel-uinput actor, true compositor libinput observer, CI-only overlay and native runner trace.
- [ ] Exact-head native CI proves SwayPX backend/press-release under disposable seatd.
- [ ] Nurse 2 G3J independent source/CI review.
- [ ] Positive OS-managed candidate from genuine device under explicit OS privilege trust model, without hardware attestation claim.
- [ ] Native guard freshness, Scroll Lock, cancellation, axis receipt, Company Map integration and operator production approval.

## NEXT

Run and inspect exact-head native CI, correct isolated fixture errors without loosening source denial. If headless + libinput session cannot function in GitHub-hosted runner, report native proof unavailable instead of claiming a passing test.

## DECISIONS

OS-managed first is accepted development direction; enrolled-device remains optional later. This milestone is a **negative known-synthetic libinput compositor callback proof**, not a physical verifier or a production scroll feature. Autoscroll HARD OFF. TimeW Restore HOLD. No merge or workstation change.
