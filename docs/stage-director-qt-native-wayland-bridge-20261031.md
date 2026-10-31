# Stage Director SD1 // Qt-owned Wayland connection bridge proof

STATE // CI-only integration feasibility, no production Qt/Quickshell plugin or browser playback.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director Weather Station integration, externally Hospital-compatible (not registered in live Hospital).

ASSIGNMENT // Operator-approved Option A NEWS-first integration. Verify public Qt6 native Wayland API can carry real mapped Weather layer and private WATCH on the **same actual client connection**.

BASE // PR #43 at `c601b870de47886fb2d9e43587f8361863772a6b`; underlying PR #42 discovery-leak fix required.

NEIGHBOR // `kudokudo1/The-Post-Apollo-Project` Weather Station worker retains all QML/Quickshell ownership.

## IMPLEMENTATION

A disposable Qt6 `QGuiApplication` obtains its existing Wayland `wl_display *` and `wl_compositor *` through the **public** `QNativeInterface::QWaylandApplication` API (Qt >= 6.5). It does not call `wl_display_connect()`, `wl_display_connect_to_fd()`, `wl_display_disconnect()`, or steal ownership of Qt's platform display. Its independently created native layer-shell role and generated private `post_apollo_weather_watch_v1` protocol objects use one borrowed display. A dedicated `wl_event_queue` keeps test-protocol event dispatch separate from Qt's QPA event handlers.

In a disposable headless CI SwayPX compositor with the PR #43 test-only scene selection, the same client maps a Weather namespace layer at the correct output coordinates and must receive the dynamically created paired-client WATCH global. It issues a known catalog ID (`bbc`), rejects a raw URL, CLOSEs and then verifies stale WATCH denial. Concurrent unpaired C clients, one with a spoofed Weather namespace, must continue to receive no global.

The test relies on the existing headless SwayPX + temporary device ACL restore scripts and produces separate logs for selected and spoofed clients. It does not edit the Weather repository, start browsers or alter the user's desktop.

The test's native layer shell is intentionally allocated by the Qt client itself, not Quickshell. **Therefore a CI pass proves Qt's API path is viable, but does not prove an actual Quickshell plugin is implemented or its window uses the expected Qt connection.** Weather's separate worker must integrate and verify this within their owning component.

## HOW TO USE

Review the draft GitHub PR and `Stage Director Qt6 same-Wayland-connection WATCH bridge (CI)` workflow. The source is in `tests/stage_director/ci_qt_native_watch_bridge.cpp` and `tests/stage_director/ci_qt_native_watch_bridge.nix`. It is explicitly blocked outside isolated GitHub CI.

The recommended Weather-side bridge is a small native C++ Qt extension, not an external `quickshell-io` Process or new Wayland socket. It should obtain `QGuiApplication::nativeInterface<QNativeInterface::QWaylandApplication>()->display()`, use generated protocol bindings and a dedicated event queue or Qt-integrated dispatch strategy, and expose QML statuses only as **request acknowledgements**, not playback. The Weather worker must reconcile Qt/Quickshell startup and display ownership and agree on how the module is packaged, loaded and given QML access. The host pairing still occurs in Stage Director, never via namespace-only self-attestation.

## VERIFY

The workflow performs source checks denying extra Wayland connections and browser/process launching, compiles the Qt6 executable with pinned Nixpkgs QtGui/QtWayland, and runs the actual headless SwayPX compositor. Success requires genuine Qt native Wayland display, mapped layer, same-connection private WATCH exchange, invalid URL rejection, CLOSE and stale denial, and continued spoof-layer invisibility. No physical keyboard, Qt Quick/Quickshell window, video or popup/seat rendering is claimed.

## WATCH OUT FOR

- Qt public Wayland native interface exists since Qt 6.5. Its availability depends on the Wayland QPA plugin actually being loaded. An X11/XWayland/fallback Qt platform is a hard fail.
- The actual Weather Quickshell process may create layer-shell windows and native extensions differently. This test intentionally cannot authenticate their `wl_client` without an integrated Weather-side bridge.
- Qt's Wayland display/socket is managed by QPA. Don't disconnect it, dispatch Qt's default event queue from another thread, or invent a separate client connection. The test uses a distinct event queue for the raw protocol proxies.
- No QtWebEngine in Quickshell. No browser, supervisor, scene/input authority, viewer startup, external URLs or local installation.
- PR #43's publication-timing security fix is mandatory; older PR #42 alone leaks private global discovery to preconnected clients, even though binding was denied.
- If the native test exposes a Qt event loop or threading conflict, preserve logs and stop; don't bypass isolation by opening another connection.

## CHECKLIST

- [x] Recheck current PRs #41-43, Weather catalog and Syntagma authority
- [x] Confirm provisional `bbc` and `cspan` source IDs exist in Weather catalog
- [x] Verify Qt public native Wayland display API and event-queue documentation
- [x] Add pinned Qt6 same-display native layer + WATCH CI test in SwayPX only
- [x] Reuse proven headless compositor and simultaneous spoofed native layers
- [x] Keep browser, UI modifications, production activation and local machine changes disabled
- [ ] Validate CI and repair faults
- [ ] Weather owner verifies native Quickshell C++ bridge and exact Wayland connection
- [ ] Add real authenticated operator input and source/lease handling
- [ ] Browser supervisor, pixel clipping, popup/focus/input containment
- [ ] Operator-authorized desktop acceptance and rollback

## NEXT

If the Qt native test passes, transfer its exact generated protocol/Qt API/queue contract to the Weather-side worker. The following SwayPX milestone can develop browser broker/supervisor with independent process and scene boundary; real viewing requires both owner lanes to join. If it fails, record exact Qt/Nix/runtime failure and repair the CI-only harness or revisit the supported Qt extension plan.

## DECISIONS

This is a draft stacked on PR #43, unmerged. Neither the Quickshell source tree, user desktop nor Hospital has been modified or registered. CI passing does not constitute final operator acceptance.
