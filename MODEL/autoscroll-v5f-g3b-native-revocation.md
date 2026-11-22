# 🏥💜 V5F-G3B // Real native target/output revocation against detached scene guard

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC; live Hospital registration not verified  
BASE // G3A [PR #164](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/164) exact `e9a615985742563245a8b662e19d2cc225ffbb49`  
OWNER BOUNDARY // Company Map [PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) remains paused and separately owned  
PERMISSION // Bounded test/docs workflow only; no local workstation modification, production activation, merge or Map code ownership transfer.

## IMPLEMENTATION

V5F-G3A introduced a **detached** timer-policy candidate. Its tests use synthetic structs and prove general invariants, but do not establish that real compositor owners actually invalidate an active guard.

G3B adds seven **new test/workflow/documentation files only**, including a C test-only probe in `tests/autoscroll/`. The CI workflow runs a disposable Nix-built SwayPX copied under `$RUNNER_TEMP`. The overlay reuses the prior C1 real target/output callback instrumentation, then TEMPORARILY (in the isolated copy ONLY):
- adds G3B probe calls after real `container_create` target bind and `container_begin_destroy` target revocation;
- adds G3B probe calls after real `output_autoscroll_config_committed` token rotation and `output_disable` output token revocation;
- links the G3A candidate and G3B probe into **the disposable SwayPX binary only**, through an isolated copied Meson file.

The G3B C probe captures **real owner-issued target and output epochs** from live SwayPX functions and actual mapped XDG objects. For a NEGATIVE policy exercise, it deliberately creates a **forged, all-positive test-only start witness**: fake physical-origin enum, fake device+seat epochs and IDs, fake verified hit test/layout state. This is NEVER a trusted native origin and MUST NEVER be promoted to production. It exists solely so the G3A cancellation function can be tested on the native owners' genuine state transitions.

The disposable test uses no real mouse or virtual input and emits no axis/frame. It launches three actual Foot XDG-shell windows, one at a time:
1. Map window A and capture a positive *test-only* baseline; commit a successful `HEADLESS-1 scale 1` reconfiguration. The actual native output epoch rotates; the embedded G3A tick MUST deny the already-started guard.
2. Map window B with a new native target token; close/unmap it. Native target revokes to zero; G3A MUST deny and permanently cancel.
3. Map window C with a new native target token; disable `HEADLESS-1`. Native output revokes to zero; G3A MUST deny and cancel before output evacuation.

All cancellations must be captured from the **real compositor process callback** and must identify which actual owner epoch changed. The pre-revocation baseline is synthetic and is not a positive authorization result.

## HOW TO USE

Only review the draft G3B PR and GitHub Actions logs. There are no desktop controls to enable, host scripts to run, packages to install, or production compositor settings to change. The harness refuses local execution. Temporary GitHub-runner `/dev/udmabuf` ACL changes are restored, even on failures.

## VERIFY

The CI must:
1. Enforce exact seven-file diff against G3A and zero production/Company changes.
2. Assert production `PA_AUTOSCROLL_LIVE_ACTIVATION 0`, native physical proof FALSE, native Scroll Lock TRUE, and bridge UNKNOWN. In the original Meson, G3A remains unlinked and the G3B test probe absent.
3. Test the overlay's source ordering: hooks strictly AFTER the actual owner token mutations and only in a temporary copy.
4. Build a real SwayPX compositor with copied Meson, G3A C and G3B probe, plus pinned Foot/Mesa on an isolated Nix runner.
5. Observe three genuine XDG maps and three distinct target owner lifetimes. Verify real output configuration epoch turnover, target epoch zero after unmap, and output epoch zero after disable.
6. Require exactly **3 synthetic-start baselines, 3 real native callbacks denying the candidate policy**, zero native policy failed logs and no axis/frame emission.
7. Require clean original checkout and runner ACL restoration.
8. Record 2/2 exact-head GitHub workflow outcomes separately from independent Nurse security review.

This test only establishes **real target/output owner invalidation**. It does not authenticate device or seat lifetimes in the same active session; prior C2 tested those separately. It does not verify real compositor scene hit-testing, a mapped target's exact corresponding display, real layout generation, native Scroll Lock, physical provenance, one four-owner atomic snapshot, live continuous scrolling, app receipt or UI.

## WATCH OUT FOR

The test C module's `PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT` is **intentionally FORGED** in a disposable test-only witness. G1 proved portable positive fields can be forged; this is expressly NOT a change to the native hard-OFF bridge. The native callback instrument does NOT have access to trusted physical event origin and G3B makes no claim otherwise.

Only target and output tokens are genuinely compositor-generated in G3B. Device+seat tokens and scene validation are test fixtures. A successful policy baseline demonstrates only that G3A can be driven with fake positive fields in a test environment. Do not call it a positive hardware, layout, four-owner or input authorization PASS.

Even a real owner token can change between a future admission check and Wayland axis/frame dispatch. Before live integration, require a reviewed compositor-owned atomic check-to-axis ordering, correct cancellation at all owner destruction paths and explicit trust model. Remember the 48ms scene age and 250ms missed-tick bounds are provisional, not UX-approved.

The original feature stack still overlaps paused Company Map #126 at older cursor/seat/Meson seams. G3B's test-only overlay changes those paths **only in a temporary GitHub runner copy**, not in GitHub's committed production source. Company retains ownership of its Room.

## CHECKLIST

- [x] Re-read current G3A and G2 exact heads, Syntagma continuity and Company Map ownership.
- [x] CI-only C owner callback probe and disposable test overlay.
- [x] Real XDG/output configuration, unmap, disable scenario harness.
- [x] Seven-file changed scope and explicit fake-start proof limits.
- [ ] Exact-head CI PASS and independent source/security review.
- [ ] Trusted physical event and device+seat producer threat model.
- [ ] Truly sampled scene/hit-test/output/layout and Scroll Lock.
- [ ] Atomic four-owner check-to-axis, real stationary scroll and UI.
- [ ] Company shared seam integration, acceptance and operator-approved merge.

## NEXT

Verify the real native CI callback logs, correct any failures on the same dedicated G3B branch without editing Company work, and report exact results. Independently review source plus synthetic start trust limits before integrating any positive producer. A future stage may test a native negative four-owner snapshot using the separately verified G2 transient-seat/virtual-pointer lifetimes, still without admitting virtual input.

## DECISIONS

**G3B = NEGATIVE REAL NATIVE TARGET/OUTPUT REVOCATION REHEARSAL ONLY.**
DRAFT · UNMERGED · PRODUCTION HARD OFF · ZERO ORIGINAL SOURCE MODIFICATIONS · NO PHYSICAL INPUT CLAIM · MAP OWNER PRESERVED.
