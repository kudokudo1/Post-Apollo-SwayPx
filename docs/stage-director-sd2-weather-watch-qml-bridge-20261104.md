# Stage Director SD2 // Opt-in Qt QML WATCH v2 client module

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station integration
ASSIGNMENT // Create standalone native QML adaptor for the real Weather client's Wayland connection
STAFF ROLE // External DOC, Hospital-compatible only
PROVIDER // OPEN AI // CHATGPT
BASE // `master` `a29b3ea9223b49559be4b24de860e56a507196c3` (PR #71 merged)
SCOPE // New `integrations/weather-watch-qml/` code, isolated CI and handoff
STATE // CI candidate; NOT installed, activated, merged or operator-accepted

## IMPLEMENTATION

The compositor's private `post_apollo_weather_watch_v1` wire protocol v2 is already implemented and tested in merged PR #71. New `PostApollo.StageDirector 1.0` Qt QML module registers `WatchBridge` as an **opt-in, disabled-by-default** client class. It uses `QGuiApplication::nativeInterface<QNativeInterface::QWaylandApplication>()->display()` to borrow the **same** native Wayland connection as Qt/Quickshell. A separate `wl_display_connect` would fail the native paired-client authorization and is forbidden. It owns an independent event queue and registry, never Qt's display socket/read side.

The private global is dynamically advertised **only after** opt-in native physical-input pairing of the Weather layer. The object binds WATCH v2, allows only fixed `bbc` or `cspan` catalog tokens and exposes `requestWatch(sourceId)`, `viewport` (chassis-local QML QRectF), `close()`, `watchCandidate`, `viewportCandidate`, `hostRevision`, `paired` and `status`. Requests can be SENT but never claim to have been ACCEPTED until the server result arrives. CANDIDATE_ONLY explicitly remains zero permission. An empty rectangle sends four zeros to revoke player geometry; close sends HIDDEN and CLOSE, and destruction tears down its reporting object.

No URL, executable path, PID, shell command, browser supervisor, scene/crop, input, popup or focus permission enters the module.

Qt QPA owns the Wayland read side. The bridge only drains its own already-queued events and flushes its outbound requests; it does not call `wl_display_read_events`, `wl_display_roundtrip` or disconnect QPA's socket. Dynamic global discovery with this cooperative pending-queue approach needs a future live Qt/Quickshell acceptance test, even though CI proves the native generated protocol and same-client Qt test fixture.

## HOW TO USE

**Source/review only** until approved for deployment:

```qml
import PostApollo.StageDirector 1.0
WatchBridge {
    id: watchBridge
    enabled: false  // deliberately OFF: host/QML owner must authorize enable
    viewport: Qt.rect(0, 0, 0, 0)
}
```

A future Weather station integration would bind `viewport` to the active SUBJECTS/MEDIA player rectangle **relative to the 1000x700 Station chassis**, set `enabled` only after the appropriate host lifecycle/explicit paired selection, call `requestWatch("bbc")` or `requestWatch("cspan")` from a real WATCH button, and call `close()` and send a zero rectangle when exiting. A successful `viewportCandidate` is **not** permission to render a video or grab pointer input.

Build CI-only Nix module with `nix build --impure --expr 'import ./integrations/weather-watch-qml/bridge.nix'` in the SwayPX checkout. It builds and executes a QML offscreen test without activating Wayland or Weather Station.

## VERIFY

- [x] Inspect current `The-Post-Apollo-Project` main and M28-V PR #1195, preserve current Weather ownership
- [x] Handoff posted to PR #1195 without changing its 54 touched files
- [x] Use canonical WATCH v2 XML from SwayPX, not a duplicated fork
- [x] No Wayland socket creation/Qt display takeover, no browser or supervisor
- [x] Offscreen QML import and disabled-by-default test authored
- [ ] Exact PR head native Qt/QML Nix build green
- [ ] Offscreen disabled default and missing-Wayland denial test green
- [ ] QtQPA same-client registry/pump event delivery in a real running Quickshell instance verified
- [ ] Real Weather SUBJECTS active viewer rectangle + UI button wired in owning Room
- [ ] Manual operator deployment/rollback/acceptance

## WATCH OUT FOR

Weather M28-V PR #1195 is actively removing the legacy root `newsViewerViewportRect`. The future active source is `StationMediaSubjectPane` → `StationSubjectNavigator` → `WeatherStationW`; importantly MEDIA overview wraps its NEWS hub inside another Item. The Weather worker owns those files and must provide a single active rectangle that goes empty on route change, hide, close, unload or terminal takeover. **Do not restore the obsolete Classic property.**

The C++ module is not automatically installed into Quickshell's QML import directory and must remain disabled absent explicit operator activation. Its asynchronous event queue uses Qt's QPA reader; CI compiles/loads the safe offscreen path, not the full live QtQPA event-loop interaction. If native event delivery does not work live, revise the event transport under a separately verified adapter, not by stealing Qt's display read-side.

## CHECKLIST

- [x] No live Fedora changes
- [x] Bounded SwayPX-owned integration, new files only
- [x] Cross-Patient collision documented and handed off
- [ ] CI green and bounded merge allowed by current operator approval
- [ ] Coordinate real Quickshell import and active-viewer property after M28-V
- [ ] Validate full WATCH/viewport/native host lifecycle on operator's desktop with separate local write/deploy approval
- [ ] Renderer and restricted browser launch stay an independent future milestone

## NEXT

Review exact-head CI, repair build/runtime failures inside this isolated Qt module, then merge only after green. Next handoff: Weather owner integrates the one active SUBJECTS/MEDIA player geometry and loads this QML module, first in disabled/read-only preflight and only later with opt-in WATCH pairing. Finally integrate the separately restricted browser process/scene/input policy.

## DECISIONS

No concurrent Weather migration edits; no direct Sway IPC/URL process spawn, no second Wayland client. The module is standalone, optional and default OFF. GitHub is source of truth, Hospital-compatible status is external/unregistered, operator final acceptance remains open.
