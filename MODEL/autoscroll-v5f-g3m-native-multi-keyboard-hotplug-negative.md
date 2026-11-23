# 🏥💜 V5F-G3M // Native two-keyboard hotplug, Scroll Lock ambiguity, HARD OFF

PATIENT // Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed native input safety  
STAFF ROLE // DOC · External Hospital-compatible (live registration unverified)  
BASE // Independently Nurse 2 reviewed G3L #190 head `014a18d89ede5c70014f2635483b832ece9ad9c2`, scoped native PASS  
PRODUCTION // Autoscroll HARD OFF · TimeW Restore HOLD · Sign-In HOLD

## IMPLEMENTATION

G3L proved one software keyboard's real native Scroll Lock keypress and observed XKB LED unchanged (`0→0`), and real single-output geometry moving `x=0→137`. Neither established system-wide lock clearance or protected layout generation. G3M addresses **the next specific gap: real second-keyboard membership changes** without inventing permission.

The disposable GitHub-hosted native SwayPX compositor COPY uses one kernel-backed software mouse, one software keyboard and a **separate second software keyboard**. All claim fake BUS_USB and are **NOT physical origin proof**. The second keyboard is created only after the first two real mouse button pairs; the runner waits for actual SwayPX `get_inputs` registration and requires that keyboard's own `KEY_SCROLLLOCK` press and release to reach SwayPX's **actual keyboard callback**. The first keyboard also sends its own real Scroll Lock pair as in G3L.

The diagnostic reads **every live keyboard device attached to the mouse's actual seat**, not merely whichever keyboard currently has focus. It records total keyboard count, identities, per-keyboard XKB LED observations, unexpected input, and keyboard group count. At middle-button callback time its native keyboard counts must be **1,1,2,1 across four press/release pairs**. Between third and fourth pairs the second kernel device is destroyed and the runner requires its real SwayPX input-manager record to disappear. Missing registration, missing key callback, wrong count or retained registration FAIL. It also preserves G3K's actual XDG target unmap and G3L's live output-layout IPC move.

Crucially, **no aggregation computes an authorized globally clear Scroll Lock state**. Even identical observed XKB LEDs are not sufficient because keymaps, groups, hotplug, virtual sources, race/freshness and hardware provenance remain unverified. Both keyboard LED observations are diagnostic and lock policy is always `UNKNOWN/DENY`. No actual scroll session exists to prove active-session cancellation. No input is consumed and no axis is emitted.

## HOW TO USE

Examine the G3M draft PR, its exact-head workflow and raw native CI log. This test runs only in a disposable GitHub-hosted Linux VM behind explicit runner/user/root/environment checks. It builds an isolated SwayPX source COPY and private seatd with real Linux `uinput` devices. The user workstation remains untouched. Kernel devices are destroyed and temporary `/dev/udmabuf` ACL restored.

## VERIFY

- **Exactly eight additive CI/test/MODEL paths** compared to reviewed G3L; zero tracked compositor runtime, seat, cursor, keyboard, output, input manager, Meson, TimeW or Company Map file changes; zero direct file overlap with paused Company Map #126.
- Real second software keyboard appears as `type=keyboard` in native `get_inputs`; two real Scroll Lock callbacks from that second device reach the native SwayPX keyboard event path.
- Real per-seat keyboard membership seen by actual native mouse callbacks is 1→2→1, then the destroyed second device is absent in actual `get_inputs`, without using fabricated flags.
- G3L still proves actual output geometry change and G3K actual mapped XDG then target unmap. All eight middle-button events continue normal seatop dispatch, guard begin/tick never authorizes, zero axes/frames.
- Unknown/conflicting LED, keyboard groups, unrecognized keyboards and no independent multi-keyboard evidence cannot become positive clearance. No new production policy input or outputs.
- All source checks and exact-head native workflow must pass before scoped technical reporting. Independent Nurse 2 exact-head review remains necessary; test-only PASS does not authorize merging parents or enabling the workstation.

## WATCH OUT FOR

G3M tests **native dynamic topology evidence**, not physically trustworthy keyboard identity. `uinput` deliberately impersonates BUS_USB; even two devices with XKB LED OFF do NOT demonstrate a globally unlocked Scroll Lock. The observed G3L LED mismatch `0→0` remains unresolved. A keyboard-group listing is only a diagnostic, not an authoritative stable lock state. Device add/remove can race with current mouse events. G3M does not provide monotonic seat/keyboard-state snapshot issuance, full multi-output layout generation, keyboard remap policy, verified hardware source, native Esc cancellation, check-before-axis freshness or real application scroll.

TimeW and C5 protected receipt work belong to other owners. Company Map #126 retains shared compositor input/scene ownership. G3G through G3L are stacked draft PRs, independently scoped-reviewed but not merged. Autoscroll compile-time/runtime remains HARD OFF.

## CHECKLIST

- [x] Reconcile G3L independent Nurse 2 scoped PASS and latest exact head.
- [x] Implement test-only second real keyboard actor, native per-seat observer, callback, trace, source checks and workflow.
- [ ] Real kernel/seat hotplug CI passes exact head and confirms 1→2→1 with callbacks/denials.
- [ ] Nurse 2 independent exact-head G3M review.
- [ ] Actual trusted multi-keyboard lock policy including keyboard groups, remap, conflicts and per-event freshness.
- [ ] Complete monotonic output-layout scene generation/ABA witness, Esc/cancel producer, real physical OS-managed source eligibility and application scroll delivery.
- [ ] Company Map seam review, safe merge sequence and explicit operator desktop activation acceptance.

## NEXT

Verify exact-head CI and correct any disposable-fixture errors without weakening input/lock denial. Then separately tackle a protected **current full layout-generation and keyboard-state freshness witness**. Production remains disabled until all gates and operator authorization.

## DECISIONS

OS-managed pointer input FIRST; optional enrolled-device layer later. G3M is strictly an isolated **negative multi-keyboard hotplug observation**. It neither grants real source trust nor enables scrolling. Autoscroll HARD OFF, TimeW Restore HOLD, Sign-In HOLD. No merge, production source changes or local workstation action.
