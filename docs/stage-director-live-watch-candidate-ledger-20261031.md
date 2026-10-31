# Stage Director SD1 // Production WATCH candidate ledger connected

STATE // Draft, production-compiled native WATCH candidate reservation and automatic revocation. No browser/scene/input authority.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director / Weather Station integration (external Hospital-compatible, no live registration verified).

ASSIGNMENT // Operator-approved Option A, NEWS viewer first, compositor-owned process and browser preparation.

BASE // PR #44 at `54aa1e5183cec006bec6bd9e80c92d385f6ac52e` (Qt native same-display headless test green).

NEIGHBOR // Weather Station QML owned by separate worker in The-Post-Apollo-Project; not edited.

## IMPLEMENTATION

Reuse the **existing** `stage_director_news_session_lease` compositor data structure rather than inventing a second supervisor. The real production `post_apollo_weather_watch_v1::watch(source_id)` handler, after exact native `wl_client`, mapped layer role, output and bound pairing generation checks, now calls a compositor-private `stage_director_news_watch_record_candidate()`. It reserves precisely one host/client/output/source/epoch candidate through the already-built ledger; duplicate WATCH requests in that epoch are **denied**, even if the second request carries a registered source ID.

The record derives host, client, output and epoch strictly from the native compositor-owned Weather host session, not from a Wayland request body. The only client field used for the ledger is the catalog source ID; it is checked against the existing fixed Weather source registry. It does not accept URLs, arguments, PIDs, browser executable paths or native pointers from QML.

The ledger is closed on explicit CLOSE, layer unmap/destroy, process/native peer invalidation and `wl_client` / role / surface destruction through the existing host session revocation callback. Its monotonically increasing epoch floor remains after clearing, preventing old reservations from being replayed. Production compositor logs include concise `WATCH_LEDGER reserved` and `WATCH_LEDGER closed` evidence, labeled `browser=OFF authority=NONE` with no URL or kernel pointer data.

**No browser is launched**. Existing supervisor already has a CI-gated fixed sleep child and PIDFD stop/reap policy, but production still has no launch issuer, and this PR does not connect one. All clip/render/input/popup/focus flags remain false. The first viewer is still pending independent containment and the Weather-side actual Quickshell bridge.

## HOW TO USE

Review this draft PR and `Stage Director live paired WATCH candidate ledger` GitHub Actions. The production code path remains unable to activate the browser. No local installation or Weather UI modification is needed. The source lives in `sway/stage_director.c`, `sway/stage_director_weather_watch_protocol.c`, and the already-existing `sway/stage_director_news_session_lease.c`.

## VERIFY

A standalone real-Wayland-socket test covers duplicate candidate denial, foreign bind, URL rejection, CLOSE, and stale request refusal. GitHub CI also compiles the normal production SwayPX binary without the separate CI-only selection hook, starts the headless CI compositor, maps a Qt6-owned Weather layer on the same native Wayland display that sends WATCH, and checks actual production compositor logs for candidate reserve and close. A simultaneously connected same-namespace spoof and unrelated layer must remain unable to discover WATCH. Temporary /dev/udmabuf ACL must be restored on exit. Checks are pending until GitHub runs this draft.

## WATCH OUT FOR

- A native host candidate is **not** proof that a real operator authorized a browser. The existing native pairing input gesture remains disabled by default, and the headless testing hook is not a physical user gesture.
- The current private WATCH protocol explicitly allows only `bbc` and `cspan` in its initial bridge contract. Weather's catalog supports both; a broader source registry may be enabled only with explicit catalog consistency and viewer constraints.
- Repeated WATCH is now **denied** until CLOSE and a fresh pairing; the client must honor the result, not presume that sending a request started playback.
- Do not merge PR #42 alone. PR #43 corrects private-global disclosure to preconnected Wayland registries. This PR depends on #44 and the entire ordered stack.
- Browser profile ownership, isolation, spawn, process group and crash recovery are not provided here. A real supervised browser is separate work.
- Existing supervisor has a CI-only fixed /usr/bin/sleep launcher, not a production browser launcher. Do not conflate it with browser readiness or release acceptance.
- No live Quickshell process or actual Weather station UI is tested here. That interface remains with a separate owner.

## CHECKLIST

- [x] Recheck canonical Syntagma rules, current SwayPX stacked PR heads and existing supervision/session/containment components
- [x] Wire production private WATCH request to existing host candidate session ledger
- [x] Derive source/host/client/output/epoch from native compositor state; no arbitrary URI
- [x] Deny duplicate WATCH and preserve binding epoch
- [x] Revoke ledger via explicit CLOSE and automatic native host/client destruction
- [x] Update live Qt6 and native Wayland duplicate-WATCH regressions
- [x] Add isolated full compositor, Qt6 bridge and protocol unit CI
- [ ] Verify GitHub checks, repair failures
- [ ] Weather worker implements actual Quickshell native bridge and in-pane WATCH controls
- [ ] Real broker/isolated browser launch, native process ownership and process-tree teardown
- [ ] Clip, popup, focus and input safety, headless pixel acceptance
- [ ] Operator-authorized local acceptance and rollback

## NEXT

After CI, use the native candidate ledger as the admission precursor to a separately reviewed browser supervisor implementation. Require verified operator gesture, actual mapped Weather host, exact session generation, private profile, browser process identity and working scene/seat containment **before** production WATCH can start a browser. Continue coordination with the Weather worker for Qt bridge incorporation into real Quickshell, not another standalone C test.

## DECISIONS

Option A remains operator-approved. This patch changes ordinary SwayPX's internal candidate ledger and logs only; it does not merge, deploy, start browsers, change Weather repo or claim live Hospital ownership. CI green is not final operator acceptance.
