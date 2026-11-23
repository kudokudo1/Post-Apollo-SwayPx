# 🏥💜 V5F-G3G // Real kernel uinput + libinput negative source experiment

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC, not verified as Hospital-connected  
BASE // G3F PR #177 head 228dae0375fb0bb18d1092d3fa41d0d17a0f1f80, independently reviewed scoped PASS  
SCOPE // CI-only kernel source test; no compositor edits  
PRODUCTION // HARD OFF

## IMPLEMENTATION

G3F's proposed OS-managed input source lane is NOT hardware attestation; G3F's proposed enrolled-device lane requires a currently nonexistent trusted issuer. A real Linux **uinput** device challenges source authentication: privileged software can create a kernel input device with real /dev/input/eventN, name and BTN_MIDDLE capabilities and claim BUS_USB and vendor/product IDs.

G3G creates one temporary real kernel uinput pseudo-mouse named PA-G3G-VIRTUAL-USB-CLAIM-NOT-HARDWARE inside a strictly guarded disposable GitHub-hosted Ubuntu runner. It advertises BUS_USB and mouse buttons/relative movement but sends **no mouse-button events**, pointer motion, axes or frames. The test discovers its actual /sys/devices/virtual/input/inputN/eventN sysfs path and matching /dev/input/eventN node from udev; passes the event path into a **real libinput path context**; verifies a pointer-capable native libinput handle with the same udev node; then calls the existing production Autoscroll kernel topology filter and origin policy without rewriting them.

Expected result: this is a **real kernel** and libinput source, but sysfs includes /virtual/, so topology_candidate=0 and the source diagnostic is DENIED. A claimed USB bus identity, pointer capability and libinput handle do **not prove** physical hardware or operator intent.

This test uses libinput directly, **not the compositor** input backend, not a SwayPX actual native button callback. G3E separately tested a real Wayland virtual-pointer button event in the compositor. Neither experiment can authorize positive mouse trust, consume a middle click, or emit scrolling.

## HOW TO USE

No new user UI is installed. Inspect exact-head GitHub workflow logs.

Safety: only a GitHub-hosted runner under an exact CI environment and RUNNER_TEMP can execute the privileged experiment. The CI runner may load the uinput module and install test dependencies within its disposable VM, never on the user's workstation. Its sole kernel test device is explicitly destroyed via UI_DEV_DESTROY even on failures. No permanent /dev permission changes, no mknod fallback, no device settings or actual input events are sent.

If the runner lacks uinput or udev/libinput cannot actually observe the test pointer, verification must **FAIL**, not report a synthetic or skipped native PASS.

## VERIFY

- Exactly FOUR new files: C native negative experiment, Python invariants, workflow and this MODEL. All checked-in production, Company Map #126 and TimeW files unchanged.
- Original production Autoscroll topology and origin-policy C are linked into the test; their implementation is not copied/modified.
- Real device created via UI_DEV_CREATE with BUS_USB claim; actual kernel sysfs and matching udev event node; native libinput pointer opened.
- Existing topology rejects real virtual kernel syspath, origin policy denies unproven source, no positive witness or scroll output.
- No HID button events transmitted to kernel; kernel test device destroyed in all cleanup paths.
- Exact-head grammar plus native CI and safety tests, independent Nurse 2 review still required.

## WATCH OUT FOR

Negative virtual-kernel detection does **not** prove all synthetic devices are rejected. A privileged synthetic HID may appear bus-backed rather than in /sys/devices/virtual. OS-managed mode still needs an operator decision about whether trusted local processes may simulate input. The enrolled-device level still has no trusted issuer and generic mice usually cannot cryptographically attest identity. Missing native Scroll Lock/layout-generation authority, check-to-axis revalidation, user-facing control, application event receipt and compositor active scrolling remain separately blocked.

TimeW transaction approval, DOC 5 protected C5 consent and Company Map input seam ownership are not part of Autoscroll G3G.

## CHECKLIST

- [x] Verify exact G3F Nurse 2 scoped PASS and Syntagma authority boundaries.
- [x] Implement kernel-uinput plus libinput negative experiment on separate test-only branch.
- [ ] Confirm actual disposable runner kernel evidence, full CI PASS and source isolation.
- [ ] Independent Nurse 2 exact-head G3G review.
- [ ] Operator agrees permitted OS-managed synthetic-input risk versus stronger enrolled trust.
- [ ] Positive native producer only after review of event authority, layout/lock freshness and guarded axis delivery.

## NEXT

Run CI. If the runner cannot produce genuine kernel input evidence, record the failure without faking a PASS. Preserve production HARD OFF.

## DECISIONS

This is a source-negative CI experiment, **not hardware attestation** or production authorization. Proposed two-tier model unchanged, no workstation mutation, no merge, Autoscroll HARD OFF.
