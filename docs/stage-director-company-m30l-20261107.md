# 🏥💜 Stage Director // M30l genuine MapLibre Native Qt v3 offline Wayland gate

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company native vector renderer integration
STAFF ROLE // Nurse 2, external Hospital-compatible (no live registration)
ASSIGNMENT // M30l build and run original Company MapLibre Qt app in isolated SwayPX
BASE // M30k PR #100 at 9039104315053a8a687784ba2c38f3dbf679f1e8
COMPANY PIN // e3595b0035bb3d89aa87c6ba2bd1aac04a975181
PERMISSION // GitHub-only; no local desktop writes, merge or production activation

## IMPLEMENTATION

M30k passed native Qt crop/input across actual Company standard and compact, hiding an 840x540 synthetic Qt source in fullscreen and reviving the SAME Wayland actor after fullscreen. Until M30l that actor was a colored QWidget, NOT a geographic renderer.

M30l starts by replacing that colored witness with the ORIGINAL Company experiments/company-vector-qt main.cpp, main.qml, CMakeLists.txt and offline synthetic GeoJSON style, built with the SwayPX flake-locked Nixpkgs MapLibre Native Qt 3.0.0 and matching QtLocation. This is a real renderer, but its offline geometry is deliberately synthetic, not real roads or clients.

The test checks the exact reviewed Company Git SHA and clean checkout, then copies only the map source to a disposable GitHub runner directory. Only that copy changes the window to 840x540 and CI-specific title, adds Qt input event and real QtLocation mapReady witnesses. Original C++ app initializes OpenGL RHI, imports QtLocation/MapLibre, defaults to local offline style and does not access owner-private data. Source asset paths relocate to immutable Nix output. No Company repo file or user's desktop is edited.

CI builds the executable through tests/stage_director/ci_company_maplibre_native.nix, launches it in NORMAL SwayPX private headless compositor and checks original native XDG app PID/pidfd, real QtLocation mapReady, actual geographic palette pixels in compositor PPM, real Wayland pointer press/drag/release/wheel, and unmap cleanup.

This first M30l gate proves real MapLibre works under native SwayPX. It does NOT yet prove the MapLibre binary above the real Company Quickshell layer in the same session. After this runtime build, a follow-on CI-only integration can adapt native actor selection and rerun M30k's dynamic viewport checks. No production rights follow from this test.

## HOW TO USE

No desktop action. Inspect the GitHub draft, Nix source build and Wayland workflow, plus screenshot artifacts. Do not install Nix test binaries on the user's Fedora host without separate verification/authorization.

## VERIFY

Source gate: exact original Company SHA, offline MapLibre API, no production bypass, CI-only source modifications.

Native gate: pinned MapLibre Native Qt version 3.0.0, original app compiled against matching Qt6, actual Wayland XDG + kernel pidfd, QML mapReady and offline geographic pixels, real Wayland pointer, app close and temporary device permissions restored.

Only green exact-head native CI counts as MapLibre runtime proof. A source green or previous M30k synthetic Qt green does not.

## WATCH OUT FOR

- The Nix MapLibre v3 package is distinct from the user's Fedora Qt6.11.2 Toolbox build. Do not copy Nix binaries to the Fedora host.
- Offline synthetic geography is NOT actual roads or verified provider coverage. Online geographic data requires opt-in and separate privacy terms.
- No authenticated production Company host/v2 fd receipt, native actor scene/input authority, fullscreen MapLibre resize, popup/focus/keyboard containment, multi-output or desktop acceptance.
- Two original Company QML coordinate issues remain unpatched in production and belong to Company's owner.
- Build, MapReady, Wayland pointer and rendered pixel failures must remain red, with precise logs.

## CHECKLIST

- [x] Re-check Syntagma, current GitHub heads, source and parallel Social owner
- [x] Inspect original Company Qt/MapLibre v3 app and offline fixture
- [x] Verify flake-pinned MapLibre package provenance
- [x] Add exact Company source-copy and Nix application derivation
- [x] Add real Wayland/XDG native screenshot and pointer runner
- [ ] Exact-head MapLibre build/runtime succeeds
- [ ] Actual MapLibre QML ready and geographic pixels verified
- [ ] Genuine map actor placed over real Company Quickshell in one CI session
- [ ] Fullscreen resizable MapLibre and real pan/zoom tests
- [ ] Production selected host + v2 receipt, independent reviews, acceptance

## NEXT

Run M30l CI and diagnose actual Nix/Qt/QML compatibility without bypassing any gate. After the renderer is proven, combine it with the real Company viewport and M30k's native input/clip contract under an isolated opt-in compositor, then tackle fullscreen.

## DECISIONS

GitHub-only external checkpoint. No live Hospital registration, no PR merge, no Company or user Fedora modifications. Do not claim a complete in-widget MapLibre until a real rendered map is actually inside the Company viewport.
