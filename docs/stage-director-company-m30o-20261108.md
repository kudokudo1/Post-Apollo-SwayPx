# 🏥💜 Stage Director // M30o actual resizable MapLibre fullscreen

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Stage Director · Company native map viewport  
STAFF ROLE // Nurse 2, external Hospital-compatible successor  
ASSIGNMENT // Genuine native MapLibre window must grow/shrink with actual Company fullscreen Canvas  
BASE // M30n PR #108 exact development HEAD `91094bf78f96eeb07a74e08985716351bf677bb6`  
COMPANY SOURCE // `e3595b0035bb3d89aa87c6ba2bd1aac04a975181`  
CONNECTION // external / not verified live Hospital registration  
PERMISSION // CI-only GitHub changes, no original Company writes, local Fedora actions, production actor activation or merge.

## IMPLEMENTATION

M30m proved that real source-pinned MapLibre displays offline synthetic geographic
pixels in original Company standard/compact Quickshell. M30n proved that
actual Wayland pan and wheel update `MapView.map.center` and `zoomLevel`,
and mode changes preserve the camera. However both intentionally **hide** the
native 840×540 source whenever the real Company fullscreen Canvas becomes
1493×788. The production engine remains uninstalled.

M30o adds an **opt-in, private CI-only exact-size profile** to the already
test-only SwayPX compositor's `sway/stage_director_ci_company_crop.c`:
- Standard/compact still require exactly 840×540 source and exact
  Quickshell-owned host geometry.
- Fullscreen requires one real 1493×788 source **and** the unique genuine
  original Company fullscreen layer with the known mapped Canvas coordinates
  `(166,203,1493,788)`.
- Any temporarily unknown, stale, or intermediate source is hidden and
  receives no map clicks. A 1200×700 source is intentionally tried as a
  **negative test**. Never grant native painting/input based on Sway IPC
  reporting a resize request was accepted.
- The compositor still verifies original Wayland client UID/kernel PID,
  process start-time and exact Nix test application path. This is not
  production identity, executable integrity or native host authorization.

The M30o runner reuses M30n's original Company QML, real MapLibre Qt v3, PIDFD,
Quickshell, camera, genuine virtual pointer and source-colored-pixel tests.
A disposable source-copy observer records **actual QML Window and MapView
pixel dimensions**. The new test resizes a single native process
840×540 → unsupported 1200×700 (hidden) → exact fullscreen 1493×788
(visible) → standard 840×540 (visible), without resetting its center/zoom.
It requires real geographic pixels and real pointer pan/wheel *in fullscreen*,
no outside-canvas source spill, outside Company clicks, stable original XDG/PID,
and clean native/Company close. M30m and M30n regressions retain their own
old source-size rejection rules with M30o OFF.

## HOW TO USE

Review `.github/workflows/stage-director-company-m30o.yml` on the stacked
M30o PR. Only a temporary GitHub Actions Nix SwayPX compositor has the
M30o flag enabled. There is no local installation or Quickshell restart.

## VERIFY

**First combined native attempt: genuine fullscreen resize, geography and
pointer PASS; standard-return source/input continuity PASS; final screenshot
threshold red.** Exact first-run
[GitHub Actions #42450476697](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42450476697)
confirmed 840×540 initial, unsupported 1200×700 hidden, actual Qt Window
*and* MapView 1493×788 with MapLibre geographic pixels visible, original
XDG/PIDFD retained, real fullscreen pan/wheel and Company outside clicks
correct, no source-overflow; 840×540 restored with exact camera preserved.

The first test incorrectly required geographic fixture pixels in the 573×369
standard viewport **after three real pan+zoom gestures had moved the actual
camera away from the finite Flint synthetic data**. The screenshot had
zero matching features, which is a valid map location outcome, not an input
or native resize failure. The revised gate does **not** recenter or assume
unlimited coverage. Instead it proves actual post-shrink QML
`Window=840×540` and `MapView=840×540`, requires
`MapView.map.mapReady=true` at every size checkpoint and verifies real
native Wayland input still changes the camera. Fullscreen real geographic
pixels remain mandatory. The subsequent rerun
[GitHub Actions #42450866267](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42450866267)
passed every real map resize, clip, input, camera and return gate, including
the positive post-shrink `MapLibre.mapReady` observer and real Wayland
input. It then missed only the **Company close timer** because M30o's QML
is scheduled to close at 122 seconds while the copied M30n final wait
expired at 26 seconds after the preceding phase. The bounded close wait
was increased to 45 seconds; the positive close/teardown check remains
mandatory. Exact-head final CI is pending.

**PENDING AT IMPLEMENTATION.** Passing M30m/M30n does not prove fullscreen
resizing. The exact native CI must establish:
1. Source boundary checks plus unchanged standard SwayPX build.
2. Real original Company QML fullscreen Canvas `1493×788`, bridge OFF.
3. Old 840×540 and intermediate 1200×700 source both hidden in fullscreen.
4. Real Qt QML Window **and MapView** become 1493×788, same XDG/PIDFD,
   then compositor logs exact profile 3.
5. Actual geographic pixels in fullscreen, real fullscreen pointer
   drag/wheel plus two Company outside-clicks, no pixel/input overflow.
6. Camera persists through *size-only* changes, then real full-screen
   navigation changes center/zoom.
7. Returning to standard hides stale 1493×788 until 840×540 is true;
   same camera/root reappear, no stale pixels on close.
8. Prior M30n/M30m exact regression and production policy remain intact.

No technical or operator acceptance before checks finish.

## WATCH OUT FOR

The geographic data is intentionally **offline synthetic Flint GeoJSON**, not
a live street map. Normal host source and Company QML belong to separate
owners. No production Company geometry correction, production trusted actor
selection, content digest, v2 PIDFD custody, permissioned user location,
network tile licensing, multiple monitors, keyboard/focus/popup containment,
or on-device Fedora acceptance is implemented in this PR.

M30o is deliberately a CI-only proof, not a replacement for production
architecture review. Actual MapLibre resizing may expose Qt/Wayland bugs;
retain failing log/screenshot artifacts and do not loosen identity or
exact-source-size checks to make CI green.

## CHECKLIST

- [x] Recheck Syntagma and current PR chain / exact M30n HEAD
- [x] Separate M30o branch stacked on M30n
- [x] Add unique exact-fullscreen CI-only compositor profile
- [x] Add temporary real Qt Window/MapView size observer
- [x] Add native size change, fail-closed negative size, camera and pointer gates
- [x] Add isolated GitHub CI and preserve M30m/M30n regressions
- [ ] M30o source and actual Wayland CI green
- [ ] Operator visual review of source-resized fullscreen screenshot
- [ ] Production host authentication/coordinate fixes and desktop acceptance

## NEXT

Run M30o CI on exact head. Diagnose actual native Qt source/geometry input
errors without weakening fail-closed policy. After a valid CI fullscreen
screenshot, review native compositor trust and Company QML geometry with
their owners. Do not activate production without explicit permission.

## DECISIONS

Fullscreen source must actually resize; using native-buffer stretching or
faking a fullscreen map is not acceptable. Size-only transitions must
preserve the real map camera. All CI-only rules are additive and gated on
`STAGE_DIRECTOR_CI_COMPANY_M30O=1`; existing M30k/M30m/M30n behavior must
remain unchanged when the flag is absent.
