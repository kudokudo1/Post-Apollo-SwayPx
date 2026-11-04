# Stage Director SD1 // Weather Station host-local viewport migration seam

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS host
ASSIGNMENT // Convert existing QML host-local NEWS player rectangle into compositor-global logical coordinates without browser authority
STAFF ROLE // DOC, external Hospital-compatible
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / NOT VERIFIED REGISTERED
BASE // `master` `fb89602498040959da6cd32db5d216fb9f11722b` (PR #69 merged)
SCOPE // SwayPX pure viewport policy, existing policy tests and handoff documentation
PERMISSIONS // Bounded GitHub source/PR/CI; NO operator Fedora or live Quickshell mutation

## IMPLEMENTATION

The verified Weather Station repository `kudokudo1/The-Post-Apollo-Project` at `main` commit `3ecfc9d16e93117b1c2a86727fd8a0cb93807833` already exposes `widgets/weather/WeatherStationW.qml::newsViewerViewportRect`, generated from `StationNewsHub.playerViewportRect` with `mapToItem(stationBackground, ...)`. These are **chassis-local** logical coordinates in a `1000x700` PanelWindow with a static `post-apollo-weather-station` overlay layer namespace. They are **not** global compositor coordinates or native independent Wayland surfaces. `newsViewerGeometryValid` only checks a nonempty inner rectangle while the legacy `currentView === "news"` route is active.

The existing SwayPX `stage_director_news_host_session` records *native* outer host bounds and output with monotonic revision / pairing epoch; `stage_director_viewport_plan` accepts global logical geometry. This change adds `stage_director_viewport_from_host_local(observed_host, inner_host_local, &global)`, a pure read-only translation and bounds check. It clears the output on denial. It does not grant launch, paint, popup, pointer, keyboard or process authority and is NOT yet connected to the client protocol.

A future trusted host adapter must obtain the current local rectangle from the **already paired same Wayland client**, map it to the compositor-observed outer geometry, ensure current epoch/geometry revision and valid global output bounds, then pass it through the existing candidate-only viewport policy. Do not invent a native `wl_surface` for an ordinary QML `Rectangle`.

## HOW TO USE

Check `Stage Director viewport safety contracts` for the standalone pure C policy test (native client process not required). The conversion is for a future **compositor-side, authenticated, read-only** NEWS viewport receipt. It is not an operator setting, a Weather QML deployment, or a browser launch API.

Migration path: `StationNewsHub.playerViewportRect` → `WeatherStationW.newsViewerViewportRect` → same-paired-client host-local geometry report (not yet implemented) → SwayPX native host/output snapshot and conversion → candidate-only viewport validation → separately authorized renderer/broker/input.

## VERIFY

- [x] Inspect current SwayPX and Weather Station heads, files and ownership
- [x] Reuse the existing viewport policy instead of duplicating another renderer/scene subsystem
- [x] Pure helper rejects empty/negative/out-of-bounds and overflowed geometry
- [x] Test stationary, moved and negative-origin multi-monitor hosts
- [ ] Exact PR head passes viewport policy, full compositor compilation and other required GitHub workflows
- [ ] Weather QML SUBJECTS/MEDIA path resolves same active NEWS viewer as legacy route
- [ ] Trusted same-Wayland-client geometry transport, revision updates and revocation
- [ ] Production browser broker, scene/input/popup handoff and local operator acceptance

## WATCH OUT FOR

`WeatherStationW.qml` and Classic-to-SUBJECTS route migration are owned in the Weather Station Patient by other workers. **Do not edit them in this Stage Director Room without a coordinated handoff.** `currentView === "news"` / `weatherContentLoader.item` is not automatically the MEDIA/SUBJECTS active viewer. The existing UI geometry is necessary but not proof of an authenticated native interior Wayland surface. `Stage Director NEWS viewport candidate` currently requires a native interior observation; that cannot be invented from a QML Rectangle. Decide the trusted same-client geometry handoff before adapting that policy.

The helper deliberately returns only geometry. It does not solve actual source attestation, popup and descendant containment, sandbox verification, production activation, or the terminal viewport's input transformation. CI-only Chrome safety proof remains distinct from a live Weather session.

## CHECKLIST

- [x] Read Syntagma admission and current repo checkpoints
- [x] Locate existing QML player local rectangle and native outer-host snapshot
- [x] Add narrow reusable local-to-global conversion with strict safety
- [x] Add policy regression test for moved/negative output/denial
- [ ] CI green, PR decision and merge evidence
- [ ] Coordinate SUBJECTS route owner and authenticated viewer geometry transport
- [ ] Do not activate any local compositor, install packages or restart Quickshell

## NEXT

Review all PR checks. If green, bounded merge into SwayPX master is allowed by current operator instruction. Then plan or implement an authenticated host-local geometry message under the existing private WATCH/native-client pairing protocol, **with the Weather owner**, before any real launcher or renderer activation. Reinspect current Weather `main` before touching it.

## DECISIONS

Use the existing Weather player viewport and existing SwayPX global viewport policy. Do not rewrite NEWS/MEDIA, manufacture an inner wl_surface identity, or enable arbitrary browser execution. No local machine writes or live Hospital registration. Operator retains final acceptance.
