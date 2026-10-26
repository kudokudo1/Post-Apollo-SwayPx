# Stage Director // Weather Station native host candidate

**STATE //** Source-only candidate inspection; no compositor activation.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`.  
**BASE //** Stacked above SD1 session PR #6 → clip PR #5 → viewport PR #4 → SD0 branch.  
**DATE //** 2026-10-25. **AUTHORITY //** Operator; no live Hospital Room/Assignment or ownership transfer presumed.

## IMPLEMENTATION

Current Weather Station NEWS uses crash-safe QtWebEngine-disabled WATCH. Browser playback must become a separate process contained inside the Weather Station viewport. SD1 PR #6 can reserve one candidate browser by epoch, host handle, PID and process-start marker, but it cannot yet identify an authorized Weather Station host.

The canonical Weather Station `widgets/weather/WeatherStationW.qml` in `kudokudo1/The-Post-Apollo-Project` is a Quickshell `PanelWindow` with `WlrLayershell.layer: WlrLayer.Overlay`, currently **without a dedicated `WlrLayershell.namespace` declaration**. The generic Quickshell layer cannot safely serve as an authenticated host.

This stage adds:

- `stage_director_weather_host.h/.c`: **candidate-only** exact namespace policy `post-apollo-weather-station`. Reject absent/wrong namespace, null native layer/output pointers, uninitialized or unmapped surface, non-overlay layer, inactive output, invalid/overflowed bounds, and host outside the compositor output. Returns `candidate_observed=true` only for a fully matching observation, with `owner_verified=false`, `can_bind_viewer=false`, `can_clip=false` always.
- `stage_director_weather_host_adapter.c`: read-only native adapter from `struct sway_layer_surface` and `struct sway_output`. Reads namespace, actual mapped status, layer type and compositor scene coordinates, and uses `output_get_box()` for logical output bounds. The adapter is **not called by SD0 callbacks or NEWS**, and it cannot start, focus or clip any window.
- Independent native C tests and a static source boundary gate, integrated with the existing Stage Director CI. No new dependencies, services or polling.

## HOW TO USE

Review/test the stacked draft PR. The native no-authority candidate policy can be compiled without launching Sway:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  sway/stage_director_viewport.c \
  sway/stage_director_weather_host.c \
  tests/stage_director/test_weather_host.c \
  -o /tmp/stage-director-weather-host-test
/tmp/stage-director-weather-host-test
python3 tests/stage_director/test_weather_host_boundary.py
```

These are examples for a disposable development environment, not instructions to change the operator's live compositor. **No local changes or production activation**.

## VERIFY

C tests cover exact namespace equality versus generic/spoofed strings, mapped/initialized/overlay/output flags, invalid pointers, secondary-output negative coordinates and host/output bounds. Static checks confirm the native adapter reads actual compositor state and has no runtime callsite or authority.

Standalone CI cannot prove the entire compositor builds or that the Weather Station host is recognized at runtime. It also cannot prove process-origin authentication, video decoding, painting, popup/input containment or safe teardown. These remain explicit acceptance gates.

## WATCH OUT FOR

- **Host not yet identifiable live.** Until a separately authorized Weather Station QML change adds the canonical namespace, this Stage Director policy must reject the current host. Do not fall back to generic `quickshell` or match the first overlay window.
- **Namespace is only classification.** Another layer-shell client may claim the same string. A future trusted compositor-side host registration must verify the native surface, origin/client, session and operator intent. Likewise, actor `app_id` and PID are not standalone authorization.
- **Geometry drift:** The actual NEWS viewer rectangle within the Station changes with layout. Its viewport cannot be inferred from the historic Kitty bay or current 1000×700 shell dimension. Capture it from the authorized UI with a verifiable association, then measure against compositor observations.
- **Output scaling:** Use `output_get_box()` to derive logical global output bounds; do not assume buffer-resolution width or DP monitor numbering.
- **No clipping activation:** This patch intentionally does not call SD1's dormant `stage_director_viewport_clip_view()`; it has no popup, focus or input policies. Do not set the clip authorization Booleans all true from matching namespace.
- **Branch chain:** PR #4 remains open against SD0; PR #5 is based on PR #4; PR #6 is based on PR #5. Do not merge this work straight to SwayPX `master` or confuse a taskbar pull with PR integration.

## CHECKLIST

- [x] Inspect repo, open PRs, SD0 lifecycle, Weather QML host and output geometry
- [x] Implement exact native host candidate extraction and fail-closed policy
- [x] Test namespace, lifecycle and output boundary
- [x] Add no-activation regression gate
- [ ] CI green
- [ ] Complete isolated SwayPX build
- [ ] Operator approves separate Weather Station namespace patch
- [ ] Trusted compositor host registration and browser-child provenance
- [ ] Popup/input guards, post-layout clip and nested compositor acceptance
- [ ] Operator merge/deployment approval

## NEXT

The next milestone requires two explicitly coordinated owners: Weather UI adds a unique layer namespace and publishes the *actual NEWS bay geometry* safely, while Stage Director implements trusted registration tying that one native layer to one verified browser child process. Only then can it attempt a reversible compositor crop under nested SwayPX with popup/pointer protection. Preserve Weather NEWS safe mode meanwhile.

## DECISIONS

No generic Quickshell-window guessing, no automatic browser launching, no promotion of a namespace or app ID to authority, no compositor/QtWebEngine activation in this stage.
