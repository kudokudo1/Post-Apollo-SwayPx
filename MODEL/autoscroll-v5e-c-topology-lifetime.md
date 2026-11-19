# SwayPX // V5E-C Kernel Topology Denial + Lifetime Issuance Readiness

**STATE:** Draft, detached lifetime issuer + read-only kernel topology negative filter. No desktop changes, no actual autoscroll enablement. Production PA_AUTOSCROLL_LIVE_ACTIVATION remains 0.

## HOSPITAL

- PATIENT // kudokudo1/Post-Apollo-SwayPx
- ROOM // Mouse & Keyboard / Universal Autoscroll V5E-C
- STAFF ROLE // External Hospital-compatible DOC; live Hospital registration not verified
- ASSIGNMENT // Continue operator-approved autoscroll work; preserve the paused Company Map worker's ownership and prior notification
- BASE // Separate stacked branch on [V5E-B PR #149](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/149), tested parent head 7f4f82cfbe09c80d74c087d94d2d9ec0bd10932f
- SCOPE // Kernel topology refusal before any libinput candidate status is set, and standalone per-object lifetime issuance. No changes to cursor.c, seat.c, Company scene, popup or default runtime activation.

## IMPLEMENTATION

### 1. Real compositor-read udev topology, strictly non-authorizing

The existing V5E-B read-only source probe now:
1. Confirms the pointer has a real wlroots libinput backend/handle.
2. Obtains a udev device reference from that exact libinput handle.
3. Reads kernel sysfs event path and /dev/input/eventN node from that same udev object.
4. Rejects missing, malformed or mismatched paths; rejects /sys/devices/virtual, uinput and uhid virtual topologies.
5. Releases the udev reference, then reads the native libinput scrolling method only if the candidate has survived the denial filter.

**Not genuine hardware proof:** Passing a nonvirtual sysfs path is NOT cryptographic or system-level hardware attestation. Some software-created HID/USB devices or compromised device stacks may appear under bus-backed paths. The source remains a candidate, not a physical device permission. A device authenticated at the backend layer is not automatically authenticated to consume middle clicks.

The portable topology policy takes *path strings*, so its unit tests are synthetic. The actual SwayPX binary build verifies the wrapper compiles with libinput and libudev; it does NOT prove physically sourced events.

### 2. Detached monotonic lifetime issuer

The portable lifetime issuer now provides the missing future source of non-repeating epoch numbers:
- A single session-local 64-bit sequence for device, seat, target and output slots
- Initial issue and invalidation, refusing duplicate binds and invalid types
- Rebinding a reused slot address mints a fresh epoch
- Refuses repeated issuer initialization and counter overflow, so no silent token reuse
- A zero value represents revoked/unavailable state

This prepares a future owner-controlled bridge to V5C PR #146's pa_autoscroll_epoch_begin/tick guard, **but does not wire it** into actual device, seat, surface or output create/destroy callbacks. A trusted compositor owner must bind and revoke tokens at those lifecycle sites and carry a distinct compositor-session identity across restarts. A synthetic guard/issuer test is not authority by itself.

### 3. Explicit ownership boundaries

Company Map's [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) remains paused/open/draft. This branch adds **no new cursor.c, seat.c or Company source changes**. The source probe change is autoscroll-owned; it adds one new pure source path to Meson, so Meson remains the **only** Company #126 changed-file overlap introduced by V5E-C. The prior shared-cursor exception lives solely in V5E-B #149, separately reviewed, unmerged and production OFF.

The inherited V5E-B CI allowlist is extended only on this child branch to include V5E-C files; parent #149 is not edited. The returning Company owner will receive a durable cross-reference in #126 after exact-head tests.

## VERIFY

- Strict portable C11 topology negative-candidate tests: pending exact-head CI
- Strict portable C11 lifetime issue/revoke/reset/overflow tests: pending
- Existing cursor ordering and hard-OFF regressions: pending
- Actual production-hard-OFF SwayPX Nix build: pending
- Physical hardware authenticity and genuine input/axis delivery: NOT TESTED

## HOW TO USE

Review GitHub source and CI only. No user desktop action.

## WATCH OUT FOR

A nonvirtual sysfs path can still represent synthetic hardware emulation. The lifetime issuer has not been attached to actual object lifetimes or session authority. The compositor runtime independently retains physical_verified=false, native_scroll_lock_active=true, layout_generation=0 and PA_AUTOSCROLL_LIVE_ACTIVATION 0. **Do not turn those gates on based on these modules.**

## CHECKLIST

- [x] Reconcile open drafts and active master, preserve parent branch ownership
- [x] Add read-only udev negative-origin filter to the private libinput candidate probe
- [x] Add portable nonrepeating lifetime issuer and replay prevention tests
- [x] Keep V5C source separate from V5E parent branches
- [x] Add isolated CI and new scope documentation
- [ ] Exact-head all relevant GitHub CI PASS
- [ ] Independent review
- [ ] Owner-managed lifecycle mint/revoke wiring across actual compositor objects
- [ ] Stronger real-device provenance and hardware vs emulated-device acceptance
- [ ] Native compositor-generated axis/frame in disposable session
- [ ] Operator-approved merge and eventual production acceptance

## NEXT

If CI is green, notify Company #126 of this child branch with its exact diff and latest head. The next authorized milestone should connect epoch issuance only where the actual owning device/seat/target/output create/unmap/destroy callbacks are controlled, and separately prove real physical-input origin without granting synthetic sources the same privilege. Preserve Weather+Company first refusal and current hard-OFF default.

## DECISIONS

Draft, stacked on V5E-B, unmerged, no local host files or processes changed, no automatic Map Room takeover. Revert by closing the child PR; no desktop rollback required.
