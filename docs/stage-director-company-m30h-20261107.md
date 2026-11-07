# 🏥💜 Hospital M30h // Real Wayland pointer delivery into cropped Qt source

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native integration
STAFF ROLE // Nurse 2, external browser worker
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE
ASSIGNMENT // M30h: verify real pointer/scroll routing and input containment
PHASE // CI-only runtime proof, operator desktop unchanged
BASE // M30g PR #91 at 50edf6924622b2c3773c98fb9677956b634751d3
AUTHORITY // Scoped GitHub development only, no merge/install/activation

## IMPLEMENTATION

M30g proved real compositor *pixel* crop of an 840×540 Qt Wayland QWidget to 600×380. That does NOT prove input hit-testing or a map that remains interactive.

M30h adds a separate disposable **Wayland zwlr_virtual_pointer_v1 sender** built from the pinned wlr-protocols XML with wayland-scanner. It sends real absolute motion, left-button press and release, drag motion with the button held, and wheel axis events to the isolated SwayPX compositor via its private Wayland socket. These travel through wlroots input, Sway seat hit-testing and Qt Wayland QPA.

The existing real Qt QPA fixture has an M30h-only application event filter that logs the **actual Qt-delivered** mouse press/move/release and wheel events, including global/local coordinates and native client PID. It records both a cyan/orange cropped map-like QWidget and a separate purple Company-shaped frame.

Input probes:
- In the visible cropped region, press (700,430), drag to (820,510), release and wheel there.
- In the Qt source's *hidden right orange overflow* at (1210,450), press, release and wheel.
- In the hidden bottom orange overflow at (720,755), press, release and wheel.
- As a control, press and release the purple frame at (400,260).
- Every pointer command must receive a real Wayland protocol sender acknowledgment and the Qt app must independently confirm the inside motion/drag/release/wheel. Pointer events outside the visible map viewport must **not** reach the map; the purple frame must receive presses at all three outside points.
- The same native processes require exact Sway XDG roots and real Linux pidfds. Real compositor screenshots must still show the 600×380 cropped pixels and no orange outside. The original purple frame must survive map close and test compositor teardown.

M30h does not enable Company production client authority and does not use real MapLibre, Quickshell Company QML, physical Company host selection, live Fedora, or production source/seat modifications.

## HOW TO USE

No operator action. Inspect this draft and the M30h "real-wayland-qt-input" job, actual compositor screenshot artifacts, and Qt receiver evidence. Existing Company → Coverage → VECTOR VIEW still opens the original standalone Qt map; default native bridge remains OFF.

## VERIFY

The source gate verifies that test-only selectors are absent from the normal SwayPX build. Runtime CI builds two compositors (normal production and test-only), two real Qt clients, the Wayland virtual pointer helper, and the pinned software renderer/screenshot client. Runtime acceptance requires actual Qt event logs and negative overflow delivery, not a focus command, a synthetic event counter in Python, or merely compositor hit-testing.

GitHub exact-head evidence is required before claiming success.

## WATCH OUT FOR

- The virtual pointer protocol is privileged on SwayPX; access is limited to this disposable unsandboxed GitHub test session. This test must NOT be installed or activated for ordinary apps.
- A pressed pointer may retain a real Wayland grab across the viewport boundary. This test releases the button *inside* before testing outside input, so it does not confuse normal dragging with unauthorized fresh clicks.
- Floating synthetic frame focus can raise the purple test window in front of the cropped map. First prove inside map delivery, then outside-to-frame delivery, then kill the map and prove frame restoration. This is NOT final layer-shell stacking.
- Source crop pixel proof does not necessarily imply hit-test bounds. The new negative overflow checks must remain red if Qt receives hidden-area events.
- Qt event dispatch may repeat a mouse event at child and parent widgets; verify presence and global positions, not exactly one event per injection.
- No direct real MapLibre movement/pan, QML pointer focus integration, popup, production authorization, keyboard capture, or performance measurement yet.

## CHECKLIST

- [x] Read latest M30g PR, check repository heads/parallel Weather owner
- [x] Add actual Wayland virtual pointer sender with isolated Nix derivation
- [x] Add Qt event filter showing real delivered mouse/wheel events
- [x] Add negative overflow click and positive drag/wheel checks
- [x] Integrate separate CI input harness without running M30g crop test twice
- [ ] Exact-head CI and actual pointer event verification
- [ ] Repair real input hit-test or Qt/Wayland failures without weakening checks
- [ ] Host-derived Company viewport geometry and authenticated native route
- [ ] Real MapLibre executable with interactive pan/zoom
- [ ] Independent review, operator merge and live desktop acceptance

## NEXT

Run the disposable runtime input workflow. Require direct Qt delivery of inside press/drag/release/wheel, and positive purple-frame response to all outside clicks with zero hidden Qt map input. Then progress toward a real Company layer hosting real MapLibre, preserving security defaults.

## DECISIONS

Operator authorized prototype-first integration. No source ownership expansion into Weather, Quickshell production changes, local workstation mutation, PR merges, or production Stage Director enablement. This report is an external Hospital-compatible checkpoint, NOT live persisted Hospital registration or final operator acceptance.
