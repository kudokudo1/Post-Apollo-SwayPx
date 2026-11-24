# 🏥💜 V5F-G3N // Test-only stale witness / keyboard-layout ABA denial

PATIENT // Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · Native-source safety / freshness  
STAFF ROLE // DOC · External, Hospital-compatible; live registration unverified  
BASE // Independently reviewed G3M #192 head `21bcce6ccd3ee3f46c3e4098864b76d6dfedaced`  
PRODUCTION // Autoscroll HARD OFF · TimeW Restore HOLD · Sign-In HOLD

## IMPLEMENTATION

G3M's **real native compositor** observed one extra kernel-software keyboard joining/leaving an already complicated seat: total keyboards **4→5→4**, three unrecognized baseline keyboards and one group; real Scroll Lock keypresses did not establish globally clear state. G3L observed an output position x=0→137, but the rectangle is not a freshness token.

G3N is a deliberately smaller **test-only, read-only-in-effect simulation** that checks a precise logical danger: **ABA**, a state temporarily changing then returning to the same apparent value. If a keyboard is attached and removed (4→5→4), or an output is moved then restored (0→137→0), the *old* witness must not become current simply because the visible counts or rectangle match again.

The standalone C test maintains untrusted local per-seat, keyboard-topology, owner and output-layout epoch counters. It records a **hypothetical test session** with a captured witness, then simulates input changes, explicitly revokes that witness when an epoch changes, and verifies it never revalidates across ABA. It also covers a same-count keyboard replacement, keyboard-group mutation, seat change, XDG owner unmap, counter overflow and invalid output layout. **The local epochs are not authenticated, not emitted by a protected compositor producer, not production state, and are not a basis to authorize real input.** The only may-consume-or-axis function is hardwired FALSE even when all simulated epochs match.

No compositor source, keyboard, cursor, layout, seat, libinput, runtime, Meson or Company Map production file is changed. No native input device is created by this new test. G3M's previously verified native evidence is its predecessor and is not replayed or inflated into a G3N native claim.

## HOW TO USE

Inspect the G3N PR exact-head GitHub CI and its C test log. No Post-Apollo menu or live desktop behavior is added. The GitHub job compiles the isolated C test and runs it as an ordinary unprivileged process; no uinput/root/seatd/GPU requirement for G3N. The existing runtime remains HARD OFF.

## VERIFY

- Four additive test/CI/model files only, on G3M reviewed head.
- Direct changed-file scope comparison with Company Map #126: zero collisions.
- Normal C11 host compilation with strict warnings as errors and direct execution.
- Eight adversarial scenarios: keyboard hotplug ABA 4→5→4, output move ABA 0→137→0, equal count but different keyboard membership, equal count but changed group, seat change, owner unmap, epoch overflow and invalid geometry.
- Every scenario must reject the prior witness and never permit consumption of a middle-button event or emission of a scroll axis. Even initially unchanged/untrusted witness is non-authorizing.
- Source regression checks preserve the checked-in production hard-off and unknown origin/layout/native Scroll Lock guard.
- Independent Nurse 2 exact-head source review required.

## WATCH OUT FOR

**Do not mistake a simulated epoch counter for a protected native issuer.** This pure C state model is an invariant test; its changes are deliberately synthetic and cannot witness real input or prove atomic before-axis cancellation. G3N does not provide keyboard ownership authentication, a globally reliable multi-keyboard Scroll Lock policy, trusted LED interpretation, remap/group semantics, source hardware attestation, monotonic current layout generation issued from real SwayPX, crash persistence or real scroll consumption. A test-only hypothetical active marker is not an actual authorized scrolling session.

The state snapshot/counter concept will require a future independently reviewed native trusted producer, protection against missing callbacks and restart/wrap/rollback, and atomic binding of the current state to the scrolling decision. If any producer fact is unknown, the outcome must be UNKNOWN/DENY. Company Map #126 retains shared production cursor/seat/scene jurisdiction and must coordinate before any real runtime changes. TimeW and C5 are separate owner scopes.

## CHECKLIST

- [x] G3M independent Nurse 2 scoped review and exact head verified.
- [x] Write additive C negative ABA and stale-revocation oracle.
- [ ] Exact-head isolated compile/source tests pass.
- [ ] Nurse 2 independent G3N review.
- [ ] Native authenticated keyboard/seat/output state generation and all-keyboard truth.
- [ ] Atomic check-before-axis, active-session revoke/cancel, real OS-managed physical source eligibility, application scroll and operator acceptance.

## NEXT

After scoped technical CI PASS and independent review, next develop a **disposable native producer observation** for real keyboard and layout signals, with fail-closed lack-of-signal behavior. Do not promote test-only counters or all-OFF LEDs to authority. No merge or desktop action.

## DECISIONS

OS-managed first, enrolled-device optional. G3N is a strictly test-only stale-witness denial model. Autoscroll HARD OFF. TimeW Restore HOLD. Sign-In HOLD.
