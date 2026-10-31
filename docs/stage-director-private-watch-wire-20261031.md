# Stage Director SD1 // Paired native WATCH/CLOSE protocol

STATE // Production-compilable protocol, fail-closed candidate responses; no live NEWS playback.
PATIENT // kudokudo1/Post-Apollo-SwayPx.
ROOM // Stage Director Weather Station integration (external Hospital-compatible, not live registered).
ASSIGNMENT // Operator-approved Option A, native pairing → private WATCH request, NEWS viewer first.
BASE // PR #41 at ecdd4981ce8d0be14750880f34d88c4cf11400b8.
NEIGHBOR // Weather Station worker independently owns The-Post-Apollo-Project QML and media/catalog.
PROVIDER // OPEN AI // CHATGPT, CONNECTION // EXTERNAL.

## IMPLEMENTATION

The ordinary SwayPX Meson build generates and compiles `post_apollo_weather_watch_v1` Wayland protocol bindings (WATCH, CLOSE, destructor, one result event). On an explicitly selected mapped Weather layer, SwayPX dynamically publishes its private global. Its `filter_global` advertises it only to that exact native `wl_client`; its bind handler verifies again. Actual WATCH and CLOSE handlers independently recheck the request `wl_resource` native client, host layer role, underlying surface, mapped state, output, current session epoch and live Linux peer evidence. A foreign client cannot bind even if it guesses the global ID.

WATCH accepts only the provisional source tokens `bbc` and `cspan` and returns `candidate_only` with all broker/render/input/seat rights false. Other input, including URLs, returns `invalid_source`. These are *bridge contract fixtures*, not a claim that the current Weather data catalog has those exact IDs or that playback works. CLOSE sends `closed` and revokes the selected client and global. Every host session invalidation, including native resource or client destruction, also revokes the dynamic global via a new optional listener; resource-lifetime tests still run independently.

No browser launching, QML change, in-process QtWebEngine, scene clipping, popup, focus or seat actuation is performed. The protocol is unavailable if `POST_APOLLO_STAGE_DIRECTOR_PAIRING` is unset or the actual physical selection ceremony has not completed. No public swaymsg/IPC method can create the pairing. Registration and request authorization are separate.

## HOW TO USE

Review the draft PR and its GitHub Actions. Do **not** install/activate this experimental SwayPX on the production desktop without a separately approved local change and rollback. The Weather Station worker must implement and test a QtWayland/Quickshell request bridge using the exact SAME Wayland client connection as the paired Weather layer; opening a separate connection will be rejected.

Protocol XML lives at `protocols/post-apollo-weather-watch-v1.xml`. Request signatures: `watch(source_id)` and `close()`; one `result(status)` event. The current statuses are `denied`, `candidate_only`, `invalid_source`, `closed`. They do NOT mean playback or session-leasing permission. The initial source IDs are test-only and must be replaced with an approved shared catalog before real viewer delivery.

## VERIFY

CI compiles the **actual production protocol implementation** with generated client/server stubs, tests a dynamically advertised paired-only global, a foreign client that cannot discover the protocol, a guessed foreign bind, known/unknown WATCH requests, CLOSE revocation and denial for previously bound stale request. Another CI job compiles the normal SwayPX binary using pinned Nix sources. Inherited PR #40/41 lifetime/identity and grammar checks should remain green. This is not proof of a physical user gesture, an actual live Qt bridge or viewer playback.

## WATCH OUT FOR

The earlier native pairing gesture remains disabled by default and is libinput-gated, not guaranteed hardware-authentic against uinput/ptrace. The selected layer must be the one the operator actually meant to select. A fake Weather layer that is genuinely clicked is not prevented merely by the namespace check. Output disconnect, role/client teardown and epoch revocation are necessary; resource/event ordering and compositor shutdown must be verified on a real headless runtime before release. The global is dynamically created to notify clients with an existing registry when pairing occurs. A bridge that has not installed its registry listener cannot discover it.

## CHECKLIST

- [x] Recheck current PR #41, stacked ownership and Syntagma authority
- [x] Add production-generated private WATCH/CLOSE protocol and named statuses
- [x] Publish only after exact native Weather selection and filter discovery
- [x] Enforce paired client independently on bind and every request
- [x] Revalidate live native host/output/session epoch and explicitly revoke on CLOSE
- [x] Revoke global automatically on client/role/surface destruction
- [x] Add real two-client forged-global-bind test and production Nix compile
- [ ] Confirm CI and repair failures
- [ ] Real headless compositor with mapped Weather layer + dynamic registry test
- [ ] Weather owner implements and proves same-wl_client Qt/Quickshell bridge
- [ ] Replace provisional source tokens with approved shared catalog
- [ ] Broker process ownership and session lease, restore/focus/render/input/popup guards
- [ ] Operator-authorized local acceptance and safe rollback

## NEXT

Once green, coordinate the client bridge with the Weather worker. The next SwayPX implementation is a real headless compositor integration test against an actual mapped layer-shell host and the new dynamic global; then the broker/viewport path. Do not claim the NEWS viewer is operational until the bridge and scene/input containment are connected.

## DECISIONS

Target Option A remains operator-approved. This slice does not merge the PR stack, modify Weather Station, restart local SwayPX, start a browser, or register in live Hospital. Technical checks are not user final acceptance.
