# 🏥 Stage Director M25 // Same-QPA Company vector client QML module

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre integration
STAFF ROLE // Nurse 2 (external, operator-authorized Company implementation owner)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // HOSPITAL-COMPATIBLE, NOT LIVE REGISTERED
ASSIGNMENT // Implement native optional Company-only QML bridge on Quickshell's exact existing Wayland connection
BASE // master 296c3fd9d9da584ba366a70cd2bd874b17d59355 (M23)
DEPENDENCY // M23 Company protocol merged; M24 physically paired Company host selection PR #79 separately in review
PHASE // Independent isolated Qt/QML package draft, no Company app import, no desktop activation

## IMPLEMENTATION

M25 introduces `integrations/company-vector-qml` with the `PostApollo.CompanyVector 1.0` QML module and `CompanyVectorBridge` type. It is modeled on the existing successful Weather QML WatchBridge transport, but binds **only** the new native Company `post_apollo_company_vector_v1` protocol and cannot access Weather WATCH.

- `enabled` defaults false; mere QML import does not connect, request, launch or use the network.
- On explicit enable, borrow `qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()->display()` from the SAME Quickshell Qt QPA process as the physically selected Company layer. Never call `wl_display_connect`, disconnect, manually read from the socket, or roundtrip inside a QML callback. Use own registry proxy/event queue; Qt QPA remains display FD owner.
- Bind the private Company global only when the compositor publishes it for the exact M24-selected Company wl_client. Merely knowing the interface name does not grant access; native registry filter/bind/request checks remain in SwayPX.
- Expose `frameRect` (Company frame inside native layer), `mapRect` (M18 active map inside Company frame), `wireBound`, `launchCandidate`, `viewportCandidate`, `epoch`, `hostRevision` and `status`.
- `requestLaunch(bool online)` sends only a fixed offline/online intent token; it never launches a Qt process, opens a browser or assumes online consent. A result explicitly distinguishes SENT from compositor candidate ACK.
- On launch candidate ACK, send the current frame/map geometry. Debounce layout changes by 25ms, send all-zero eight-field report on hidden/invalid QML geometry and on close. Local nested rectangle policy refuses nonfinite, negative, overflowing and map-outside-frame values.
- Respond to global withdrawal and protocol close by clearing candidate state; no auto-reconnect authority or stale epoch reuse.
- CMake generates client code from **the single SwayPX M23 XML**, builds a Qt6 QML module and runs an offscreen smoke test with default OFF, known rectangles, disabled intent rejection and explicit non-Wayland fail-closed.
- Pinned-Nix `bridge.nix` compiles, packages and tests the actual installed Qt QML import, without using operator desktop libraries or modifying Quickshell.

This source is **not imported by CompanyNetworkW.qml yet**. It does not enable M24, authenticate the native Qt MapLibre actor, or embed pixels. That later integration must map the actual M18 Company frame and map rect through this module and verify its exact Qt Wayland connection.

## HOW TO USE

GitHub CI is the only active M25 usage. On the operator desktop, Company → Coverage → VECTOR VIEW still opens a separate native Qt map window. No commands, packages, production services, app menu imports, or local file writes are required now. The QML type can be used later only after an explicitly reviewed native module install and opt-in Company QML wiring.

## VERIFY

- Current SwayPX M23 XML, native server request statuses and Weather's proven same-QPA Qt bridge source inspected.
- CI checks no second Wayland connection, duplicate protocol XML, process creation, scene, seat, focus or popups.
- Native QML offscreen test imports the compiled module, verifies it is disabled, verifies existing frame/map properties, refuses unpaired launch and fails closed on non-Wayland Qt.
- Nix check tests QML import again from the installed package path, not only source.
- No real user Wayland/QPA combined Company physical selection test yet, no native MapLibre XDG actor custody, cropping or input proof.

## WATCH OUT FOR

- `wireBound` means the Company M23 interface object exists for this Wayland client. It is not equivalent to authorization to launch a map, identity of the Qt XDG actor, or permission to crop/receive input.
- The online intent value is an ACKed transport token only; actual network tiles remain behind the separate existing Company UI online consent.
- QML `frameRect` must be relative to the native PanelWindow layer, whereas M18 `mapRect` is relative to `companyFrame`. The correct QQuickItem transform must be verified in Company before wiring.
- Importing and enabling this QML module requires the plugin installed in the same Qt ABI/environment as the running Quickshell, not an unrelated Toolbox Qt version. No user-local install was attempted.
- The plugin's default off and non-Wayland refusal do not prove a full live Qt QPA event loop; a later isolated mapped Company layer + real client test is needed.

## CHECKLIST

- [x] M23 native Company-only private Wayland protocol merged
- [x] Dedicated CompanyVectorBridge QML module using existing Qt QPA display
- [x] Fixed offline/online intent and bounded nested map/frame geometry report
- [x] Disabled-by-default, revoked-global cleanup, hidden/close behavior
- [x] Isolated CMake Qt module and pinned-Nix package with offscreen test
- [x] M25 source/packaged QML CI submitted
- [ ] Exact-head M25 CI green and independently reviewed
- [ ] M24 physical selector PR #79 green/merged
- [ ] Controlled Company QML wiring with verified import and map chassis reflow
- [ ] Trusted M15 Qt actor launch provenance/scene custody
- [ ] Pixel/crop/resize/input/focus/popup acceptance and safe rollback
- [ ] Operator final live acceptance

## NEXT

Verify M25 native QML package CI and M24 full SwayPX regressions independently. Land each under the delegated Company implementation authority only when exact head is green. Build an isolated actual same-QPA Company QML client test through M24 physical pairing, then integrate one optional Company UI bridge that derives `frameRect` and `mapRect` from real M18 QQuickItem map geometry. Avoid installing system plugin or restarting Quickshell until a separate reversible operator-local activation checkpoint.

## DECISIONS

M25 changes only SwayPX `integrations/company-vector-qml`, its CI and recovery documentation. No Company repo/QML modifications or compositor actor privileges. External Nurse 2, no live Hospital registration.
