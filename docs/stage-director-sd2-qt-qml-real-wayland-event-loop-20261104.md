# 🏥💜 Stage Director SD2 // Real Qt-owned QML WATCH event loop proof

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station WATCH integration
ROLE // External DOC / Hospital-compatible, no verified live Hospital registration
ASSIGNMENT // Verify the installed `WatchBridge` QML module receives real private WATCH v2 messages through the same Qt-QPA-owned Wayland connection that maps a Weather host
BASE // `master` `8d07a451a7eaaa84c12dcd8d4cb28cf58538a4af`, PR #72 landed
SCOPE // CI fixture, Nix package, workflow, recovery notes; no operator workstation mutation
STATE // Verification PR draft, exact-head checks pending

## IMPLEMENTATION

Previous PR #72 compiled the disabled-by-default `PostApollo.StageDirector 1.0` QML plugin and verified its packaged import in an **offscreen** Qt process. PR #71 exercised WATCH v2 against a **native C++ fixture** borrowing Qt's existing display. Neither proves that the *actual QML module* receives messages from Qt's asynchronous event loop.

This CI-only fixture adds a separate compile variant of the existing `ci_qt_native_watch_bridge.cpp`:
- `QGuiApplication` owns the sole Wayland connection; native Qt QPA exposes it via `QWaylandApplication`.
- Fixture maps a real `post-apollo-weather-station` layer shell on that same connection.
- A `QQmlEngine` loads the **installed** `PostApollo.StageDirector 1.0` `WatchBridge`, with `enabled: true` only inside a disposable runner.
- The compositor's CI-only deterministic scene hit-test selects exactly that mapped native Weather layer, after spoof and unrelated peer layers are present. Production physical-input pairing remains opt-in.
- A bounded `QGuiApplication::processEvents` loop, with **no test-native roundtrip or raw WATCH calls after the QML instance is created**, must deliver the dynamic WATCH registry announcement and the module's own Wayland v2 callback.
- The actual QML properties are read through `QObject`; the test checks fixed-catalog denial, BBC candidate, positive viewport + nonzero host revision, viewer hidden, a second current viewport, CLOSE acknowledgement, and disabled-only denial. A status-only or native-only fixture cannot pass this test.
- The smoke script requires both a real QML receipt marker and the ordinary mapped-host WATCH ledger, including exact client/spoof rejection. It remains limited to a GitHub Actions / Nix store / disposable XDG runtime.

No WeatherStationW.qml, Quickshell, source data, filesystem state or production SwayPX selection is changed. No browser process is launched, and the test's "candidate" never becomes scene, pointer, keyboard or popup authority.

## HOW TO USE

GitHub Actions workflow: `Stage Director real Qt QML WATCH bridge on paired Wayland host (isolated)`. The workflow builds non-CI compositor, independent CI-only compositor, mapped layer test actor, installed QML plugin, and alternate QtQML client fixture, then runs `ci_udmabuf_smoke.sh` in a temporary CI runtime.

The operator need not install, restart or click anything on their Fedora desktop. The successful CI job is a **protocol event-delivery verification**, not usable video playback.

## VERIFY

- [x] Recheck Syntagma Hospital rules and current SwayPX `master` and open PR ownership
- [x] Preserve existing v1/v2 Qt native fixture when alternate flag is absent
- [x] Add actual installed QML engine import and Qt-QPA event loop test
- [x] Require real mapped Weather layer, current host revision, valid/hidden/reported transitions, CLOSE and disable
- [x] Guard against new native display, browser process, compositor scene authority
- [ ] Exact-head Python boundary job passes
- [ ] Exact-head Nix package/native Qt QML build passes
- [ ] Exact-head isolated SwayPX QtQPA event delivery and full smoke pass
- [ ] Green workflow review and bounded merge
- [ ] Operator live Quickshell session pairing and UI acceptance

## FIRST-RUN FINDING // Real QPA CLOSE lifecycle

The **first exact-head** integration job reached `WATCH_CANDIDATE_ONLY`, `VIEWPORT_CANDIDATE_ONLY`, hidden, and renewed viewport responses on the **real installed QML object**, but failed on CLOSE because SwayPX sends `CLOSED` then deliberately revokes the private global. The client `registryRemoved` callback then overwrote `CLOSED` with `PAIRING_REVOKED`.

Resolution in this branch: `WatchBridge` remembers only a **server-acknowledged** CLOSE, and preserves `CLOSED` when its subsequent global removal arrives. An unacknowledged/unexpected removal continues to emit `PAIRING_REVOKED`. The real QPA fixture now requires `paired == false`, `watchCandidate == false`, `viewportCandidate == false`, and terminal `status == CLOSED` before passing. This was a genuine client lifecycle mismatch, not a server authority bypass or QML import failure.

## WATCH OUT FOR

The existing QML module's `wl_display_dispatch_queue_pending` relies on Qt QPA to read messages from the native display while the Qt GUI event loop runs. Offscreen import cannot prove this; this fixture must detect and reject failure. It must **not** take over Qt's socket reader or call `wl_display_roundtrip_queue` after creating the QML object. If a real Qt event-loop issue is exposed, change the client transport with a reviewed nonblocking integration, not the compositor's pairing security.

CI-only scene selection isn't a real pointer gesture. A QML `Rectangle` remains data, not a native inner Wayland window. The production NEWS player, browser process, clipping, input, popup lifecycle and actual runtime deployment all remain separate. The Weather SUBJECTS migration and header TERMINAL restoration are owned by the Weather Patient and must not be changed here.

## CHECKLIST

- [x] No local workstation or production process changes
- [x] One QPA-owned Wayland connection
- [x] Actual installed `WatchBridge` QML instance, no test-native WATCH substitute
- [x] Existing process, scene, and runtime guards retained
- [ ] CI fully green before any merge
- [ ] SwayPX master merge commit and recovery pointer
- [ ] Weather active MEDIA viewer geometry and real Quickshell module import after owner coordination

## NEXT

After CI is green, merge only the tested SwayPX CI slice. If it fails, diagnose the specific failing QPA registry/event queue transition in runner logs and patch the client adapter with strict no-second-reader behavior. With proven delivery, coordinate the active SUBJECTS/MEDIA player's local rect and `WatchBridge` binding in Weather's own Room. Restricted browser launch/clip/input remains gated by separate operator acceptance and runtime checks.

## DECISIONS

External Hospital-compatible DOC, no verified live registration. GitHub-only test work authorized by CONTINUE; no local writes, production activation, bulk PR merge or cross-Patient Weather QML mutation. Preserve original Qt fixture as comparison control. Operator holds final acceptance.
