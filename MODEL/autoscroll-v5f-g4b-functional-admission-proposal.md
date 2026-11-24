# 🏥💜 G4B // Functional activation and shared-input contract (PROPOSED)

PATIENT // Post-Apollo-SwayPx
ROOM // Universal Autoscroll
STAFF // successor Autoscroll DOC; live Hospital registration unverified
BASE // G4A #204 0253a9353d0569894a92d9c5c53b95dd2ef000d7, 24/24 exact-head workflows PASS
ADJACENT OWNER // Company Map #126 3d0f7ce7d0b2774ad58ce0169ba78f68ff035d0c
PRODUCTION // HARD OFF

## IMPLEMENTATION

This is a decision proposal, not a grant or implementation of native activation. G4A integrates the tested hard-deny machinery. G3F's two-tier model remains the basis. Do not label OS-managed input as authenticated physical input.

### Proposed activation contract

OFF (default): no runtime allocation, timers, input capture or emitted axes.
OS-managed (initial functional lane, pending operator ratification): explicit local seat configuration selection; trust the running compositor and local OS input boundary, not hardware authenticity. Require real compositor-owned libinput pointer callback, actual live device attachment, coherent nonvirtual kernel topology, nonconflicting native button-scroll method, and exact owner epochs. Reject protocol virtual inputs, known kernel uinput paths, incomplete evidence, stale events, touch/tablet emulation, session lock, pointer constraints and conflicting grabs. A privileged synthetic HID that resembles a physical device is not cryptographically distinguishable; this residual risk must be accepted before enabling this mode. Do not set the existing PHYSICAL_LIBINPUT enum or trusted_physical_pointer boolean merely from candidate facts. Introduce a separately named policy-scoped admission contract; preserve legacy portable tests and review migration of positive fields.
Enrolled-device: unavailable until a reviewed protected issuer, scoped enrollment/revocation and event binding exist. No invented credential, serial-number authentication or automatic upgrade.

### Session and cancellation

Use compositor-owned event sequence and current session generation with nonzero owner-issued device, seat, target and output epochs. All admission occurs synchronously in native callback after owner refusal. Store no unchecked raw pointers as persistent identity; resolve live objects by owner records before dereference. Require direct mapped target and exact focused surface/root correspondence, stable layout and scene epoch, no pending transaction or conflicting grab. Refuse embedded Company surfaces in initial functional slice. Runtime owns one session per seat; no global synthetic session and no cross-seat scroll. Explicitly check current facts before each axis and frame; do not fabricate a fresh pointer timestamp from timer clock. Separate admission event freshness from ongoing compositor-owned live continuity with bounded lifetime and timer gaps; review new contract before altering existing guard. Wheel/motion intent goes only to the captured still-valid target, not whichever surface became focused. Cancel before teardown, owner change, focus/layout/group change, session lock, constraint, mode change, Scroll Lock, Esc or conflicting grab. Re-enable never revives old session. Pair consumed press/release exactly by live device and seat epoch; revoke outstanding pairing on lifetime loss, never forward an orphan release or swallow unrelated release. Validate client receipts, not just seatop logs.

### Proposed per-seat Scroll Lock

Separate autoscroll feature mode (default OFF) from per-seat armed/disarmed state (starts disarmed on uncertain state). Use a compositor-owned toggle, not a borrowed XKB LED. One accepted, debounced native keyboard press toggles state; release/repeat do not toggle and aggregate group callback must not duplicate device callback. Any observed disable request cancels current session immediately; unknown source or ambiguous state never enables. A trusted local re-enable action requires fresh policy checks, never resumes previous session. Operator decision required for exact keyboard trust, persistence and whether external virtual keyboard can request disable-only. Native libinput button scrolling retains ownership when configured; do not change libinput configuration. Esc cancels an active session without changing the persistent preference; define paired key release behavior and preserve other keyboard shortcuts. Indicator noninteractive, scoped to active session, no focus grab, no activation authority.

### Shared Company Map boundary

Compare #204 and #126 actual patches: seat.h and seat.c lifetime changes versus separate Company focus declarations/guards; Meson source list additions; cursor.c overlap immediately after Weather/Company pairing. Required order: existing pairing guards -> Company captured-release and hit/focus containment decisions -> Autoscroll admission or ordinary dispatch. Company-owned native grab/embedded actor is excluded from initial Autoscroll; do not bypass its motion/axis containment, keyboard focus guard, or held release. Timer emission must recheck same ownership and current focused target rather than directly notifying the seat around Company axis gate. Coordinate merge order with owner. No edits to #126; if both sides need integration, create separately reviewed combined candidate after agreement. TimeW #199 has no direct file overlap, but scene transactions still invalidate active session. Preserve TimeW restore and Sign-In HOLD.

## VERIFY

G4A integrated GitHub exact head 24/24 workflows PASS; raw cumulative log: 24/24 normal callbacks, 5/5 shadow cancellations, zero real axes. G4A independent review pending; G3Q/R separate reviews pending. Existing Company regression CI is not a combined #126 runtime test. No approval inferred.
Acceptance gates for G4C/D/E:
1. Document operator OS-managed risk choice and independent permission review.
2. Portable model and native private callback proof; no positive physical claim; known synthetic denial. Test-only positive fixture distinctly scoped and not installable.
3. Native isolated client receipt for actual vertical/horizontal scroll to exact target under explicit disposable configuration; zero foreign target deliveries and no input loss when OFF. Verify lock/escape, held release, hotplug, reload, focus, owner epoch, target destruction and output change.
4. Combined owner regression with Company active native scene, no map/Weather pairing steal, no unexpected pointer/axis routing.
5. Separately authorized desktop trial on operator's mouse, Brave/terminal, multiple monitors; explicit opt-in, visible indicator, immediate rollback to OFF. No automatic live install or production activation.

## NEXT / DECISIONS

Review this proposal; confirm initial OS-managed threat boundary and keyboard semantics before positive native implementation. Shared owner reconciliation and independent integrated review are outstanding. Source-only proposal leaves all production code untouched. No merge, no registration claim, no workstation mutation. Autoscroll HARD OFF.
