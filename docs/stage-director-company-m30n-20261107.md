# 🏥💜 Stage Director // M30n real MapLibre camera and geography continuity

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Stage Director · Company native map renderer  
STAFF ROLE // Nurse 2 · external Hospital-compatible successor  
ASSIGNMENT // Require real QtLocation camera state and visible offline geography after pan/zoom/mode changes  
BASE // [M30m PR #107](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/107), merged into M30l development branch at `89634947fd054988a2ac80f5503108a0beac09ec`  
COMPANY SOURCE // exact `The-Post-Apollo-Project` commit `e3595b0035bb3d89aa87c6ba2bd1aac04a975181`  
CONNECTION // external; no live Hospital registration verified  
PERMISSIONS // GitHub CI-only development. No local files/desktop restarts, original Company writes, production activation or merge approval assumed.

## IMPLEMENTATION

M30m proves real MapLibre Native Qt v3 inside the original Company Quickshell
PanelWindow, exact standard/compact geometry, clipped Wayland pointer and
fullscreen hide/return with the same real XDG/PIDFD actor. But those real
press/drag/wheel witnesses do **not** prove the map changes geographical
center/zoom, and the standard-return screenshot after the larger gestures
does not show all original synthetic roads. The M30l offline style is a
*small synthetic GeoJSON region near Flint*, not nationwide tiles; panning
out of that region can correctly leave a blank background.

M30n adds only a **read-only camera observer to the disposable CI source
copy** in `ci_company_m30n_prepare.py`. The original CI source guards,
original Company Git checkout, original MapLibre v3 renderer, offline style,
no-owner-data and native input instrumentation are retained. The new QML
properties expose `MapView.map.center.latitude`, `.longitude`, and
`.zoomLevel`. A Qt timer samples those *real* QML properties into public
fixture-only CI logs; it never writes or resets the camera.

The M30n runner preserves M30m's private compositor, original Company
source, genuine Qt surface and all exact clip/pidfd/pointer/negative-input
tests. It uses shorter **real Wayland drags** so the intentionally small
offline fixture remains within range, and separately verifies:

1. Initial map center is near the public Flint fixture and zoom near 10.
2. Actual Wayland press/held-motion/release changes QtLocation camera
   center, not just a mouse-event log.
3. Actual Wayland wheel changes QtLocation zoom, not just an event log.
4. Real offline cyan/blue/orange geographic pixels are present *after*
   navigation, excluding the QML header/footer.
5. Company standard→compact and compact→fullscreen-hidden and return
   do not move or reset the camera without a map gesture.
6. The real same PID/XDG actor, hiding, outside Company clicks and close
   behavior remain the M30m invariants.
7. Returned standard viewport contains real geographic pixels, including
   after one more real pan/zoom.

A missing map camera change, hidden input leak, camera reset, missing
geographic features, or lost native identity is a test failure. Don't
declare the navigation path complete merely because CI receives wheel
and pointer events.

## HOW TO USE

Review the new `.github/workflows/stage-director-company-m30n.yml`.
The workflow runs only on an ephemeral GitHub Actions private 1920×1080
Wayland/SwayPX compositor with a source-pinned offline MapLibre build.
It never launches the live Company widget or calls production backends.

## VERIFY

**Pending at implementation.** A red new camera test is evidence of a
real missing behavior or a test-assumption mismatch, not authorization to
remove safety checks. Required exact-head success evidence:
- Source-only boundary gates and normal SwayPX untouched
- Original Company, M30l MapLibre and M30m CI viewport clipping
- Native camera QML properties change due to real drag *and* wheel
- Camera stays stable over layout-only changes, fullscreen hidden, and return
- Actual geographic raster pixels remain in standard viewport after
  navigation/return, and hidden full-screen source is absent
- Correct native XDG/PIDFD, no outside-pointer leak, safe close and ACL reset

## WATCH OUT FOR

This is synthetic offline geography, not real roads or provider coverage.
The finite Flint fixture does not authorize global or online tile requests.
A camera may legitimately reach empty geography after sufficiently large
pan/zoom; M30n deliberately bounds gestures to avoid confusing that with
renderer failure. We must not silently change actual Company camera state,
recenter a user’s view, or interpret empty tiles as company coverage loss.

CI-only source-copy instrumentation is not a production feature. True
source-resizable fullscreen MapLibre, a selected authenticated native host,
original Company QML coordinate repairs, keyboard/focus/popup isolation,
multi-output scaling, enterprise privacy and real Fedora acceptance are
separate ownership and authorization gates.

## CHECKLIST

- [x] Recheck Syntagma and current M30k/M30l/M30m merged development state
- [x] Create M30n branch on exact M30m stacked-merge commit
- [x] Add real read-only QML camera observer in disposable source copy
- [x] Add actual Wayland pan, zoom, camera-preservation and geography gates
- [x] Preserve M30m regression/production admission defaults
- [ ] Exact-head M30n source and native CI successful
- [ ] Operator screenshot approval
- [ ] Next: resizable native map fullscreen, source-owner QML fix, custody review

## NEXT

Run exact-head GitHub Actions; inspect camera and pixel evidence before
claiming a working navigation path. Keep code in draft PR until the technical
gates pass. Continue toward a source-resizable full Company map and
reviewed production integration, but do not deploy to the operator desktop.

## DECISIONS

Camera state is owned by the real QtLocation MapView. Company chassis
transitions should not change that camera just to hide/clip it. The test
does not automatically reset a user's geographic position, enable online
tiles or claim native Company production custody. Operator remains final
authority for review, merge and live acceptance.
