# Stage Director SD1 // Weather WATCH v2 read-only player viewport receipt

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station native viewport integration
STAFF ROLE // External DOC, Hospital-compatible; no verified live registration
ASSIGNMENT // Receive one active Weather player rectangle from the existing physically paired Wayland client without activating browser, compositor scene or input
BASE // `master` `211f14f098a18c8f01dff0a57b5a38ec132f061f` (merged PR #70)
STATE // Candidate branch / CI verification required, no local deployment

## IMPLEMENTATION

The existing private `post_apollo_weather_watch_v1` interface is versioned to **2** while preserving the existing v1 `watch`, `close`, `destroy` and `result` ABI. v2 introduces `report_viewport(x,y,width,height)` with **chassis-local logical coordinates** and a `viewport_result(status,host_revision_hi,host_revision_lo)` event. Status codes are `DENIED=0`, `CANDIDATE_ONLY=1`, `HIDDEN=2`. `(0,0,0,0)` explicitly clears the player candidate.

The client must first physically pair the actual Weather layer (the opt-in compositor-controlled pairing ceremony), bind to the same native `wl_client` on WATCH v2, and reserve an existing allowlisted WATCH candidate. Every viewport message rechecks the bound resource/client, pairing epoch, live compositor-observed layer and output, and current native WATCH lease. Foreign, stale, malformed or out-of-host geometry is denied.

The actual native outer host geometry is owned by `stage_director_news_host_session`. This code stores a **candidate-only** global rectangle computed by the pure host-local translation introduced in PR #70, with the compositor's geometry revision. A layer move, resize, output transfer, unmap, resource loss, CLOSE or fresh selection invalidates stale candidate custody. A malformed rectangle from the real paired client clears its previous candidate, but a forged foreign request cannot clear it.

The reply echoes the compositor-observed host revision. Every `can_launch`, `can_clip`, `can_render`, `can_accept_input` and `can_show_popup` bit remains false, including on a valid candidate. The protocol does not accept an executable, URL, PID, scene node, native surface pointer, output name or generic public IPC.

## HOW TO USE

Protocol implementers: on the same real Qt/Wayland connection that owns `WeatherStationW.qml`'s layer-shell surface, bind the private interface at v2 only after the compositor's native pairing. Send WATCH with an allowlisted catalog source, then send `report_viewport` when the active player rectangle changes. Send `0,0,0,0` on viewer close, unload or route switch. Read the `viewport_result` event and never interpret `CANDIDATE_ONLY` as permission to show video. Legacy v1 clients still have WATCH/CLOSE functionality and cannot send v2 report messages.

The correct Weather source is `StationNewsHub.playerViewportRect`, converted by `WeatherStationW.newsViewerViewportRect` to **chassis-local** coordinates. However, current `WeatherStationW.qml` only obtains that data through the inactive legacy NEWS loader. SUBJECTS/MEDIA has its own active loader. The Weather owner must expose a shared active-viewer rectangle before building the client emitter. Do **not** manufacture a Wayland inner surface: a QML Rectangle isn't one.

## VERIFY

- [x] Confirm SwayPX and Weather Patient heads and existing worker ownership
- [x] Extend WATCH protocol with v1 backward-compatibility and v2 geometry-only receipt
- [x] Apply native peer/host/lease gates before processing any client rectangle
- [x] Store candidate with native revision, revoke after movement and host teardown
- [x] Exercise foreign client, invalid bounds, hide, stale revision and zero-permission behavior in native host tests
- [x] Add static ABI/authority boundary assertions in native-host CI
- [ ] Exact head passes native-resource-lifetime, viewport policy, full compositor build and headless checks
- [ ] Actual wire-level v2 request/response from the same real Qt Wayland connection
- [ ] SUBJECTS/MEDIA active-viewer source correction with Weather owner
- [ ] Production browser supervisor/renderer/input, local operator acceptance

## WATCH OUT FOR

The server receives geometry reports but **there is no Weather v2 client sender wired yet**. The new message has no ability to launch Chrome or activate media. Client reports are *data*, not evidence of real video ownership or an authenticated native inner Wayland surface. The native host is observed separately; only a future trusted compositor session may decide to use the receipt after installing every necessary popup, input, frame and process guard.

Keep test-only Chrome first-frame controls out of the production binary, and do not enable process spawn or alter QML on another worker's active migration branch. User has allowed bounded GitHub merges and edits. No live Fedora action was performed.

## CHECKLIST

- [x] No local desktop changes, SwayPX server-side development only
- [x] v1 compatibility preserved, v2 candidate/hidden/denied statuses explicit
- [x] Host-revision handoff and read-only candidate storage
- [ ] CI green and merge exact tested head if permitted
- [ ] Integrate Qt/QML emitter with route owner and perform same-client wire integration test
- [ ] Next isolated real Chrome-generated descendant test and/or restricted browser integration as separately verified scope

## NEXT

Coordinate the single `activePlayerViewportRect` in Weather SUBJECTS and the native same-`wl_display` Qt bridge. The next distinct milestone is end-to-end `WATCH → VIEWPORT_RESULT` real-wire confirmation using the active Weather layer, without a browser. After that, implement the restricted single-browser supervisor/scene broker behind explicit operator activation and rollback.

## DECISIONS

SwayPX owns native authority and global host bounds. Weather Station owns QML route state and local active-player bounds. The protocol carries only four local integers, not a process or scene credential. No production browser activation, no local machine changes, no bulk merge of legacy Stage Director experimental branches. Operator retains final acceptance.
